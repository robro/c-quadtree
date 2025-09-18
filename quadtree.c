#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"

#define QT_DEFAULT_CAPACITY 8
#define QT_NODE_CAPACITY 10

struct QuadTreeNode {
	uint point_count;
	struct AABB boundary;
	struct Vec2 *points[QT_NODE_CAPACITY];
	int child_indices[4];
};

struct QuadTree {
	uint size;
	uint capacity;
	struct QuadTreeNode *nodes;
};

void quadtree_node_init(struct QuadTreeNode *node, struct AABB *boundary) {
	node->point_count = 0;
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
	assert(index >= 0);
	struct QuadTreeNode *node = &qtree->nodes[index];

	if (!aabb_contains_point(&node->boundary, point)) {
		return false;
	}
	if (node->point_count < QT_NODE_CAPACITY && node->child_indices[0] < 0) {
		// we have room for more points and no children yet
		// assume if any child indices are invalid they all are
		// so just add the point here and increment point count
		node->points[node->point_count++] = point;
		return true;
	}
	if (node->child_indices[0] < 0 && qtree->size > qtree->capacity - 4) {
		// realloc the nodes to fit more points (double capacity)
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
		// we don't have room for more points and need to subdivide
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
	// add new point to whichever child will accept it
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

uint quadtree_node_points_in_range(struct QuadTree *qtree, int index, struct AABB *range) {
	assert(index >= 0);
	struct QuadTreeNode *node = &qtree->nodes[index];
	
	if (node->point_count == 0) {
		return 0;
	}
	if (!aabb_intersects_range(&node->boundary, range)) {
		return 0;
	}
	uint points_in_range = 0;
	for (int i = 0; i < node->point_count; ++i) {
		points_in_range += aabb_contains_point(range, node->points[i]);
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
