#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"
#include "util.h"

#define QT_DEFAULT_CAPACITY 8
#define QT_NODE_CAPACITY 10

typedef struct {
	uint entity_count;
	Rect boundary;
	void *entities[QT_NODE_CAPACITY];
	int child_indices[4];
} QuadTreeNode;

struct QuadTree {
	uint size;
	uint capacity;
	QuadTreeNode *nodes;
};

void quadtree_node_init(QuadTreeNode *node, const Rect *boundary) {
	node->entity_count = 0;
	node->boundary = *boundary;
	node->child_indices[0] = -1;
}

QuadTree* quadtree_new(const Rect *boundary) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	QuadTree *qtree = malloc(sizeof(*qtree));
	if (qtree == NULL) {
		return NULL;
	}
	QuadTreeNode *nodes = malloc(sizeof(*nodes) * QT_DEFAULT_CAPACITY);
	if (nodes == NULL) {
		free(qtree);
		return NULL;
	}
	qtree->size = 1;
	qtree->capacity = QT_DEFAULT_CAPACITY;
	qtree->nodes = nodes;
	quadtree_node_init(&qtree->nodes[0], boundary);
	return qtree;
}

void quadtree_clear(QuadTree *qtree) {
	qtree->size = 1;
	quadtree_node_init(&qtree->nodes[0], &qtree->nodes[0].boundary);
}

void quadtree_free(QuadTree *qtree) {
	free(qtree->nodes);
	free(qtree);
}

uint quadtree_get_size(QuadTree *qtree) {
	return qtree->size;
}

typedef bool IntersectsFunc(const void *, const void *);

bool _rect_intersects_point(const void *rect, const void *point) {
	return rect_intersects_point(rect, point);
}

bool _rect_intersects_rect(const void *rect1, const void *rect2) {
	return rect_intersects_rect(rect1, rect2);
}

bool _rect_intersects_circle(const void *rect, const void *circle) {
	return rect_intersects_circle(rect, circle);
}

bool _rect_intersects_entity_rect(const void *rect, const void *entity_rect) {
	return rect_intersects_entity_rect(rect, entity_rect);
}

bool _rect_intersects_entity_circle(const void *rect, const void *entity_circle) {
	return rect_intersects_entity_circle(rect, entity_circle);
}

bool _circle_intersects_circle(const void *circle1, const void *circle2) {
	return circle_intersects_circle(circle1, circle2);
}

bool _entity_circle_intersects_entity_circle(const void *entity_circle_1, const void *entity_circle_2) {
	return entity_circle_intersects_entity_circle(entity_circle_1, entity_circle_2);
}

bool _entity_rect_intersects_entity_rect(const void *entity_rect_1, const void *entity_rect_2) {
	return entity_rect_intersects_entity_rect(entity_rect_1, entity_rect_2);
}

bool quadtree_node_add_entity(QuadTree *qtree, int index, void *entity, IntersectsFunc intersects_node) {
	QuadTreeNode *node = &qtree->nodes[index];

	if (!intersects_node(&node->boundary, entity)) {
		return false;
	}
	if (node->entity_count < QT_NODE_CAPACITY) {
		// we have room for more entities and no children yet
		// assume if any child indices are invalid they all are
		// so just add the entity here and increment entity count
		node->entities[node->entity_count++] = entity;
		return true;
	}
	if (node->child_indices[0] < 0 && qtree->size > qtree->capacity - 4) {
		// realloc the nodes to fit more entities (double capacity)
		QuadTreeNode *new_nodes = realloc(
			qtree->nodes, sizeof(*new_nodes) * qtree->capacity * 2
		);
		if (new_nodes == NULL) {
			printf("ERROR: Failed to allocate new memory! Can't add point!\n");
			return false;
		}
		qtree->nodes = new_nodes;
		qtree->capacity *= 2;
		node = &new_nodes[index];
		printf("QuadTree node capacity doubled to: %d nodes\n", qtree->capacity);
	}
	if (node->child_indices[0] < 0) {
		// we don't have room for more entities and need to subdivide
		for (int i = 0; i < 4; ++i) {
			node->child_indices[i] = qtree->size + i;
		}

		Vec2 boundary_center = rect_get_center(&node->boundary);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(Rect){
			.min = node->boundary.min,
			.max = boundary_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(Rect){
			.min = {.x = boundary_center.x, .y = node->boundary.min.y},
			.max = {.x = node->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(Rect){
			.min = {.x = node->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = node->boundary.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(Rect){
			.min = boundary_center,
			.max = node->boundary.max,
		});
		qtree->size += 4;
	}
	// add new entity to whichever child will accept it
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_entity(qtree, node->child_indices[i], entity, intersects_node)) return true;
	}
	printf("ERROR: Reached unreachable code!\n");
	return false;
}

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int count) {
	for (int i = 0; i < count; ++i) {
		quadtree_node_add_entity(qtree, 0, &points[i], _rect_intersects_point);
	}
}

void quadtree_add_rects(QuadTree *qtree, Rect *rects, int count) {
	for (int i = 0; i < count; ++i) {
		quadtree_node_add_entity(qtree, 0, &rects[i], _rect_intersects_rect);
	}
}

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int count) {
	for (int i = 0; i < count; ++i) {
		quadtree_node_add_entity(qtree, 0, &circles[i], _rect_intersects_circle);
	}
}

uint quadtree_add_entities_rect(QuadTree *qtree, EntityRect *rects, int count) {
	uint entities_added = 0;
	for (int i = 0; i < count; ++i) {
		entities_added += quadtree_node_add_entity(qtree, 0, &rects[i], _rect_intersects_entity_rect);
	}
	return entities_added;
}

uint quadtree_add_entities_circle(QuadTree *qtree, EntityCircle *circles, int count) {
	uint entities_added = 0;
	for (int i = 0; i < count; ++i) {
		entities_added += quadtree_node_add_entity(qtree, 0, &circles[i], _rect_intersects_entity_circle);
	}
	return entities_added;
}

void quadtree_node_entities_intersecting_entity(const QuadTree *qtree, int index, const void *entity, void *results, IntersectsFunc intersects_node, IntersectsFunc intersects_entity) {
	QuadTreeNode *node = &qtree->nodes[index];
	if (node->entity_count == 0) {
		return;
	}
	if (!intersects_node(&node->boundary, entity)) {
		return;
	}
	for (int i = 0; i < node->entity_count; ++i) {
		if (entity == node->entities[i]) {
			continue;
		}
		if (intersects_entity(entity, node->entities[i])) {
			dynamic_array_push_back(results, node->entities[i]);
		}
	}
	if (node->child_indices[0] < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		quadtree_node_entities_intersecting_entity(qtree, node->child_indices[i], entity, results, intersects_node, intersects_entity);
	}
}

void quadtree_points_intersecting_rect(const QuadTree *qtree, const Rect *rect, DynamicArray *results) {
	quadtree_node_entities_intersecting_entity(qtree, 0, rect, results, _rect_intersects_rect, _rect_intersects_point);
}

void quadtree_rects_intersecting_rect(const QuadTree *qtree, const Rect *rect, DynamicArray *results) {
	quadtree_node_entities_intersecting_entity(qtree, 0, rect, results, _rect_intersects_rect, _rect_intersects_rect);
}

void quadtree_circles_intersecting_circle(const QuadTree *qtree, const Circle *circle, DynamicArray *results) {
	quadtree_node_entities_intersecting_entity(qtree, 0, circle, results, _rect_intersects_circle, _circle_intersects_circle);
}

void quadtree_entities_circle_intersecting_entity_circle(const QuadTree *qtree, const EntityCircle *entity_circle, DynamicArray *results) {
	quadtree_node_entities_intersecting_entity(qtree, 0, entity_circle, results, _rect_intersects_entity_circle, _entity_circle_intersects_entity_circle);
}

void quadtree_entities_rect_intersecting_entity_rect(const QuadTree *qtree, const EntityRect *entity_rect, DynamicArray *results) {
	quadtree_node_entities_intersecting_entity(qtree, 0, entity_rect, results, _rect_intersects_entity_rect, _entity_rect_intersects_entity_rect);
}
