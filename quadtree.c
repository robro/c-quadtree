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

void quadtree_node_init(QuadTreeNode *node, Rect *boundary) {
	node->entity_count = 0;
	node->boundary = *boundary;
	node->child_indices[0] = -1;
}

QuadTree* quadtree_new(Rect *boundary) {
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

typedef bool IntersectsFunc(void *, void *);

bool _rect_intersects_point(void *rect, void *point) {
	return rect_intersects_point(rect, point);
}

bool _rect_intersects_rect(void *rect1, void *rect2) {
	return rect_intersects_rect(rect1, rect2);
}

bool _rect_intersects_circle(void *rect, void *circle) {
	return rect_intersects_circle(rect, circle);
}

bool _circle_intersects_circle(void *circle1, void *circle2) {
	return circle_intersects_circle(circle1, circle2);
}

bool quadtree_node_add_shape(QuadTree *qtree, int index, void *shape, IntersectsFunc func) {
	QuadTreeNode *node = &qtree->nodes[index];

	if (!func(&node->boundary, shape)) {
		return false;
	}
	if (node->entity_count < QT_NODE_CAPACITY && node->child_indices[0] < 0) {
		// we have room for more entities and no children yet
		// assume if any child indices are invalid they all are
		// so just add the entity here and increment entity count
		node->entities[node->entity_count++] = shape;
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
		node = &qtree->nodes[index];
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
		if (quadtree_node_add_shape(qtree, node->child_indices[i], shape, func)) return true;
	}
	printf("ERROR: Reached unreachable code!\n");
	return false;
}

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count) {
	for (int i = 0; i < point_count; ++i) {
		quadtree_node_add_shape(qtree, 0, &points[i], _rect_intersects_point);
	}
}

void quadtree_add_rects(QuadTree *qtree, Rect *rects, int rect_count) {
	for (int i = 0; i < rect_count; ++i) {
		quadtree_node_add_shape(qtree, 0, &rects[i], _rect_intersects_rect);
	}
}

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count) {
	for (int i = 0; i < circle_count; ++i) {
		quadtree_node_add_shape(qtree, 0, &circles[i], _rect_intersects_circle);
	}
}

typedef void PushBackFunc(void *, void *);

void _point_array_push_back(void *array, void *point) {
	point_array_push_back(array, point);
}

void _rect_array_push_back(void *array, void *rect) {
	rect_array_push_back(array, rect);
}

void _circle_array_push_back(void *array, void *circle) {
	circle_array_push_back(array, circle);
}

void quadtree_node_shapes_intersecting_shape(QuadTree *qtree, int index, void *shape, void *results, IntersectsFunc intersects_node, IntersectsFunc intersects_shape, PushBackFunc push_back) {
	QuadTreeNode *node = &qtree->nodes[index];
	if (node->entity_count == 0) {
		return;
	}
	if (!intersects_node(&node->boundary, shape)) {
		return;
	}
	for (int i = 0; i < node->entity_count; ++i) {
		if (intersects_shape(shape, node->entities[i])) {
			push_back(results, node->entities[i]);
		}
	}
	if (node->child_indices[0] < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		quadtree_node_shapes_intersecting_shape(qtree, node->child_indices[i], shape, results, intersects_node, intersects_shape, push_back);
	}
}

void quadtree_points_intersecting_rect(QuadTree *qtree, Rect *rect, PointArray *results) {
	quadtree_node_shapes_intersecting_shape(qtree, 0, rect, results, _rect_intersects_rect, _rect_intersects_point, _point_array_push_back);
}

void quadtree_rects_intersecting_rect(QuadTree *qtree, Rect *rect, RectArray *results) {
	quadtree_node_shapes_intersecting_shape(qtree, 0, rect, results, _rect_intersects_rect, _rect_intersects_rect, _rect_array_push_back);
}

void quadtree_circles_intersecting_circle(QuadTree *qtree, Circle *circle, CircleArray *results) {
	quadtree_node_shapes_intersecting_shape(qtree, 0, circle, results, _rect_intersects_circle, _circle_intersects_circle, _circle_array_push_back);
}
