#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include "quadtree.h"

void quadtree_node_init(struct QuadTreeNode *node, struct AABB *boundary, uint depth) {
	node->boundary = *boundary;
	node->depth = depth;
	node->point_count = 0;
	node->children[0] = NULL;
}

void quadtree_clear(struct QuadTree *qtree) {
	quadtree_node_init(qtree->nodes, &qtree->boundary, 0);
	qtree->node_count = 1;
}

struct QuadTree *quadtree_new(struct AABB *boundary) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	struct QuadTree *qtree = malloc(sizeof(*qtree));
	if (qtree == NULL) {
		return NULL;
	}
	qtree->boundary = *boundary;
	quadtree_clear(qtree);
	return qtree;
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
	if (node->children[0] == NULL && qtree->node_count > QT_MAX_NODES - 4) {
		printf("ERROR: Cannot add more nodes! Node limit: %d\n", QT_MAX_NODES);
		return false;
	}
	if (node->children[0] == NULL) {
		// we don't have room for more points and need to subdivide
		for (int i = 0; i < 4; ++i) {
			node->children[i] = &qtree->nodes[qtree->node_count + i];
		}
		qtree->node_count += 4;

		struct Vec2 boundary_center = aabb_get_center(&node->boundary);

		quadtree_node_init(node->children[0], &(struct AABB){
			.min = node->boundary.min,
			.max = boundary_center,
		}, node->depth + 1);
		quadtree_node_init(node->children[1], &(struct AABB){
			.min = {.x = boundary_center.x, .y = node->boundary.min.y},
			.max = {.x = node->boundary.max.x, .y = boundary_center.y}
		}, node->depth + 1);
		quadtree_node_init(node->children[2], &(struct AABB){
			.min = {.x = node->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = node->boundary.max.y}
		}, node->depth + 1);
		quadtree_node_init(node->children[3], &(struct AABB){
			.min = boundary_center,
			.max = node->boundary.max,
		}, node->depth + 1);
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
