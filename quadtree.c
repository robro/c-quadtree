#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"
#include "util.h"

#define QT_DEFAULT_CAPACITY 8
#define QT_NODE_CAPACITY 10

struct QuadTreeNode {
	uint entity_count;
	struct AABB boundary;
	void *entities[QT_NODE_CAPACITY];
	int child_indices[4];
};

struct QuadTree {
	uint size;
	uint capacity;
	struct QuadTreeNode *nodes;
};

void quadtree_node_init(struct QuadTreeNode *node, struct AABB *boundary) {
	node->entity_count = 0;
	node->boundary = *boundary;
	node->child_indices[0] = -1;
}

struct QuadTree* quadtree_new(struct AABB *boundary) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	struct QuadTree *qtree = malloc(sizeof(*qtree));
	if (qtree == NULL) {
		return NULL;
	}
	struct QuadTreeNode *nodes = malloc(sizeof(*nodes) * QT_DEFAULT_CAPACITY);
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

void quadtree_clear(struct QuadTree *qtree) {
	qtree->size = 1;
	quadtree_node_init(&qtree->nodes[0], &qtree->nodes[0].boundary);
}

void quadtree_free(struct QuadTree *qtree) {
	free(qtree->nodes);
	free(qtree);
}

bool quadtree_node_add_point(struct QuadTree *qtree, int index, struct Vec2 *point) {
	struct QuadTreeNode *node = &qtree->nodes[index];

	if (!aabb_contains_point(&node->boundary, point)) {
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
		struct QuadTreeNode *new_nodes = realloc(
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

		struct Vec2 boundary_center = aabb_get_center(&node->boundary);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(struct AABB){
			.min = node->boundary.min,
			.max = boundary_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(struct AABB){
			.min = {.x = boundary_center.x, .y = node->boundary.min.y},
			.max = {.x = node->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(struct AABB){
			.min = {.x = node->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = node->boundary.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(struct AABB){
			.min = boundary_center,
			.max = node->boundary.max,
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

void quadtree_add_points(struct QuadTree *qtree, struct Vec2 *points, int point_count) {
	assert(point_count >= 0);
	for (int i = 0; i < point_count; ++i) {
		quadtree_node_add_point(qtree, 0, &points[i]);
	}
}

bool quadtree_node_add_circle(struct QuadTree *qtree, int index, struct Circle *circle) {
	struct QuadTreeNode *node = &qtree->nodes[index];

	if (!aabb_intersects_circle(&node->boundary, circle)) {
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
		struct QuadTreeNode *new_nodes = realloc(
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

		struct Vec2 boundary_center = aabb_get_center(&node->boundary);

		quadtree_node_init(&qtree->nodes[node->child_indices[0]], &(struct AABB){
			.min = node->boundary.min,
			.max = boundary_center,
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[1]], &(struct AABB){
			.min = {.x = boundary_center.x, .y = node->boundary.min.y},
			.max = {.x = node->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[2]], &(struct AABB){
			.min = {.x = node->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = node->boundary.max.y}
		});
		quadtree_node_init(&qtree->nodes[node->child_indices[3]], &(struct AABB){
			.min = boundary_center,
			.max = node->boundary.max,
		});
		qtree->size += 4;
	}
	// add new entity to all children that will accept it (it can be in multiple)
	// it should always be accepted unless something weird has happened
	bool accepted = false;
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_circle(qtree, node->child_indices[i], circle)) accepted = true;
	}
	if (!accepted) {
		printf("ERROR: Circle not added for some reason!\n");
	}
	return accepted;
}

void quadtree_add_circles(struct QuadTree *qtree, struct Circle *circles, int circle_count) {
	assert(circle_count >= 0);
	for (int i = 0; i < circle_count; ++i) {
		quadtree_node_add_circle(qtree, 0, &circles[i]);
	}
}

uint quadtree_node_points_in_range(struct QuadTree *qtree, int index, struct AABB *range) {
	struct QuadTreeNode *node = &qtree->nodes[index];
	
	if (node->entity_count == 0) {
		return 0;
	}
	if (!aabb_intersects_range(&node->boundary, range)) {
		return 0;
	}
	uint points_in_range = 0;
	for (int i = 0; i < node->entity_count; ++i) {
		points_in_range += aabb_contains_point(range, node->entities[i]);
	}
	if (node->child_indices[0] < 0) return points_in_range;
	for (int i = 0; i < 4; ++i) {
		points_in_range += quadtree_node_points_in_range(qtree, node->child_indices[i], range);
	}
	return points_in_range;
}

uint quadtree_points_in_range(struct QuadTree *qtree, struct AABB *range) {
	return quadtree_node_points_in_range(qtree, 0, range);
}

uint quadtree_node_circles_intersecting_circle(struct QuadTree *qtree, int index, struct Circle *circle) {
	struct QuadTreeNode *node = &qtree->nodes[index];
	
	if (node->entity_count == 0) {
		return 0;
	}
	if (!aabb_intersects_circle(&node->boundary, circle)) {
		return 0;
	}
	uint circles_intersecting_circle = 0;
	for (int i = 0; i < node->entity_count; ++i) {
		if (circle == node->entities[i]) continue;
		circles_intersecting_circle += circle_intersects_circle(circle, node->entities[i]);
	}
	if (node->child_indices[0] < 0) return circles_intersecting_circle;
	for (int i = 0; i < 4; ++i) {
		circles_intersecting_circle += quadtree_node_circles_intersecting_circle(qtree, node->child_indices[i], circle);
	}
	return circles_intersecting_circle;
}

uint quadtree_circles_intersecting_circle(struct QuadTree *qtree, struct Circle *circle) {
	return quadtree_node_circles_intersecting_circle(qtree, 0, circle);
}
