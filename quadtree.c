#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"
#include "util.h"

#define QT_DEFAULT_CAPACITY 8
#define QT_NODE_CAPACITY 10

typedef struct {
	uint entity_count;
	Range2 range;
	void *entities[QT_NODE_CAPACITY];
	int child_indices[4];
} QuadTreeNode;

struct QuadTree {
	uint size;
	uint capacity;
	QuadTreeNode *nodes;
};

void quadtree_node_init(QuadTreeNode *node, Range2 *range) {
	node->entity_count = 0;
	node->range = *range;
	node->child_indices[0] = -1;
}

QuadTree* quadtree_new(Range2 *range) {
	assert(range->min.x < range->max.x && range->min.y < range->max.y);
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
	quadtree_node_init(&qtree->nodes[0], range);
	return qtree;
}

void quadtree_clear(QuadTree *qtree) {
	qtree->size = 1;
	quadtree_node_init(&qtree->nodes[0], &qtree->nodes[0].range);
}

void quadtree_free(QuadTree *qtree) {
	free(qtree->nodes);
	free(qtree);
}

bool quadtree_node_add_point(QuadTree *qtree, int index, Vec2 *point) {
	QuadTreeNode *node = &qtree->nodes[index];

	if (!range_contains_point(&node->range, point)) {
		return false;
	}
	if (node->entity_count < QT_NODE_CAPACITY && node->child_indices[0] < 0) {
		// we have room for more entities and no children yet
		// assume if any child indices are invalid they all are
		// so just add the entity here and increment entity count
		node->entities[node->entity_count++] = point;
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

		Vec2 range_center = range_get_center(&node->range);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(Range2){
			.min = node->range.min,
			.max = range_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(Range2){
			.min = {.x = range_center.x, .y = node->range.min.y},
			.max = {.x = node->range.max.x, .y = range_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(Range2){
			.min = {.x = node->range.min.x, .y = range_center.y},
			.max = {.x = range_center.x, .y = node->range.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(Range2){
			.min = range_center,
			.max = node->range.max,
		});
		qtree->size += 4;
	}
	// add new entity to whichever child will accept it
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_point(qtree, node->child_indices[i], point)) return true;
	}
	printf("ERROR: Reached unreachable code!\n");
	return false;
}

bool quadtree_node_add_range(QuadTree *qtree, int index, Range2 *range) {
	QuadTreeNode *node = &qtree->nodes[index];

	if (!range_intersects_range(&node->range, range)) {
		return false;
	}
	if (node->entity_count < QT_NODE_CAPACITY && node->child_indices[0] < 0) {
		// we have room for more entities and no children yet
		// assume if any child indices are invalid they all are
		// so just add the entity here and increment entity count
		node->entities[node->entity_count++] = range;
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

		Vec2 range_center = range_get_center(&node->range);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(Range2){
			.min = node->range.min,
			.max = range_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(Range2){
			.min = {.x = range_center.x, .y = node->range.min.y},
			.max = {.x = node->range.max.x, .y = range_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(Range2){
			.min = {.x = node->range.min.x, .y = range_center.y},
			.max = {.x = range_center.x, .y = node->range.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(Range2){
			.min = range_center,
			.max = node->range.max,
		});
		qtree->size += 4;
	}
	// add new entity to all children that will accept it (it can be in multiple)
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_range(qtree, node->child_indices[i], range)) return true;
	}
	printf("ERROR: Reached unreachable code!\n");
	return false;
}

bool quadtree_node_add_circle(QuadTree *qtree, int index, Circle *circle) {
	QuadTreeNode *node = &qtree->nodes[index];

	if (!range_intersects_circle(&node->range, circle)) {
		return false;
	}
	if (node->entity_count < QT_NODE_CAPACITY && node->child_indices[0] < 0) {
		// we have room for more entities and no children yet
		// assume if any child indices are invalid they all are
		// so just add the entity here and increment entity count
		node->entities[node->entity_count++] = circle;
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

		Vec2 range_center = range_get_center(&node->range);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(Range2){
			.min = node->range.min,
			.max = range_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(Range2){
			.min = {.x = range_center.x, .y = node->range.min.y},
			.max = {.x = node->range.max.x, .y = range_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(Range2){
			.min = {.x = node->range.min.x, .y = range_center.y},
			.max = {.x = range_center.x, .y = node->range.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(Range2){
			.min = range_center,
			.max = node->range.max,
		});
		qtree->size += 4;
	}
	// add new entity to all children that will accept it (it can be in multiple)
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_circle(qtree, node->child_indices[i], circle)) return true;
	}
	printf("ERROR: Reached unreachable code!\n");
	return false;
}

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count) {
	for (int i = 0; i < point_count; ++i) {
		quadtree_node_add_point(qtree, 0, &points[i]);
	}
}

void quadtree_add_ranges(QuadTree *qtree, Range2 *ranges, int range_count) {
	for (int i = 0; i < range_count; ++i) {
		quadtree_node_add_range(qtree, 0, &ranges[i]);
	}
}

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count) {
	for (int i = 0; i < circle_count; ++i) {
		quadtree_node_add_circle(qtree, 0, &circles[i]);
	}
}

void quadtree_node_points_in_range(QuadTree *qtree, int index, Range2 *range, PointArray *results) {
	QuadTreeNode *node = &qtree->nodes[index];
	if (node->entity_count == 0) {
		return;
	}
	if (!range_intersects_range(&node->range, range)) {
		return;
	}
	for (int i = 0; i < node->entity_count; ++i) {
		if (range_contains_point(range, node->entities[i])) {
			point_array_push_back(results, node->entities[i]);
		}
	}
	if (node->child_indices[0] < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		quadtree_node_points_in_range(qtree, node->child_indices[i], range, results);
	}
}

void quadtree_node_ranges_intersecting_range(QuadTree *qtree, int index, Range2 *range, Range2Array *results) {
	QuadTreeNode *node = &qtree->nodes[index];
	if (node->entity_count == 0) {
		return;
	}
	if (!range_intersects_range(&node->range, range)) {
		return;
	}
	for (int i = 0; i < node->entity_count; ++i) {
		if (range == node->entities[i]) {
			continue;
		}
		if (range_intersects_range(range, node->entities[i])) {
			range_array_push_back(results, node->entities[i]);
		}
	}
	if (node->child_indices[0] < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		quadtree_node_ranges_intersecting_range(qtree, node->child_indices[i], range, results);
	}
}

void quadtree_node_circles_intersecting_circle(QuadTree *qtree, int index, Circle *circle, CircleArray *results) {
	QuadTreeNode *node = &qtree->nodes[index];
	if (node->entity_count == 0) {
		return;
	}
	if (!range_intersects_circle(&node->range, circle)) {
		return;
	}
	for (int i = 0; i < node->entity_count; ++i) {
		if (circle == node->entities[i]) {
			continue;
		}
		if (circle_intersects_circle(circle, node->entities[i])) {
			circle_array_push_back(results, node->entities[i]);
		}
	}
	if (node->child_indices[0] < 0) {
		return;
	}
	for (int i = 0; i < 4; ++i) {
		quadtree_node_circles_intersecting_circle(qtree, node->child_indices[i], circle, results);
	}
}

void quadtree_points_in_range(QuadTree *qtree, Range2 *range, PointArray *results) {
	quadtree_node_points_in_range(qtree, 0, range, results);
}

void quadtree_ranges_intersecting_range(QuadTree *qtree, Range2 *range, Range2Array *results) {
	quadtree_node_ranges_intersecting_range(qtree, 0, range, results);
}

/*
 * Finds Circles in QuadTree overlapping input Circle and returns result in CircleArray.
 */
void quadtree_circles_intersecting_circle(QuadTree *qtree, Circle *circle, CircleArray *results) {
	quadtree_node_circles_intersecting_circle(qtree, 0, circle, results);
}

