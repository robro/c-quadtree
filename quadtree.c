#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"

void quadtree_node_init(struct QuadTreeNode *node, struct AABB *boundary) {
	node->boundary = *boundary;
	node->point_count = 0;
	node->children[0] = NULL;
}

void quadtree_clear(struct QuadTree *qtree) {
	qtree->size = 1;
	quadtree_node_init(qtree->nodes, &qtree->nodes[0].boundary);
}

bool quadtree_init(struct QuadTree *qtree, struct AABB *boundary) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	struct QuadTreeNode *nodes = malloc(sizeof(*nodes) * QT_DEFAULT_CAPACITY);
	if (nodes == NULL) {
		return false;
	}
	qtree->size = 1;
	qtree->capacity = QT_DEFAULT_CAPACITY;
	qtree->nodes = nodes;
	quadtree_node_init(qtree->nodes, boundary);
	return true;
}

bool quadtree_node_add_point(struct QuadTreeNode *node, struct Vec2 *point, struct QuadTree *qtree) {
	if (!aabb_contains_point(&node->boundary, point)) {
		return false;
	}
	if (node->point_count < QT_NODE_CAPACITY && node->children[0] == NULL) {
		// we have room for more points and no children yet (assume if any children are NULL they all are)
		// so just add the point here and increment point count
		node->points[node->point_count++] = point;
		return true;
	}
	if (node->children[0] == NULL && qtree->size > qtree->capacity - 4) {
		// realloc the nodes to fit more points (double capacity)
		struct QuadTreeNode *new_nodes = realloc(
			qtree->nodes, sizeof(*new_nodes) * qtree->capacity * 2
		);
		if (new_nodes == NULL) {
			printf("Failed to allocate new memory! Can't add point!\n");
			return false;
		}
		printf("QuadTree node capacity doubled\n");
		qtree->nodes = new_nodes;
		qtree->capacity *= 2;
	}
	if (node->children[0] == NULL) {
		// we don't have room for more points and need to subdivide
		for (int i = 0; i < 4; ++i) {
			node->children[i] = &qtree->nodes[qtree->size + i];
		}
		qtree->size += 4;

		struct Vec2 boundary_center = aabb_get_center(&node->boundary);

		quadtree_node_init(node->children[0], &(struct AABB){
			.min = node->boundary.min,
			.max = boundary_center,
		});
		quadtree_node_init(node->children[1], &(struct AABB){
			.min = {.x = boundary_center.x, .y = node->boundary.min.y},
			.max = {.x = node->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_node_init(node->children[2], &(struct AABB){
			.min = {.x = node->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = node->boundary.max.y}
		});
		quadtree_node_init(node->children[3], &(struct AABB){
			.min = boundary_center,
			.max = node->boundary.max,
		});
	}
	// add new point to whichever child will accept it
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_node_add_point(node->children[i], point, qtree)) return true;
	}
	return false;
}

void quadtree_add_points(struct QuadTree *qtree, struct Vec2 *points, uint point_count) {
	for (int i = 0; i < point_count; ++i) {
		quadtree_node_add_point(qtree->nodes, &points[i], qtree);
	}
}

uint quadtree_node_points_in_range(struct QuadTreeNode *node, struct AABB *range) {
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
	if (node->children[0] == NULL) return points_in_range;
	for (int i = 0; i < 4; ++i) {
		points_in_range += quadtree_node_points_in_range(node->children[i], range);
	}
	return points_in_range;
}

uint quadtree_points_in_range(struct QuadTree *qtree, struct AABB *range) {
	return quadtree_node_points_in_range(qtree->nodes, range);
}
