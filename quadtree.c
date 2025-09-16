#include <assert.h>
#include <stdio.h>

#include "quadtree.h"

uint qtree_count = 0;
uint point_count = 0;

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point) {
	if (point->x < boundary->min.x || point->x >= boundary->max.x ||
		point->y < boundary->min.y || point->y >= boundary->max.y) {
		return false;
	}
	return true;
}

bool aabb_intersects_range(struct AABB *boundary, struct AABB *range) {
	if (boundary->max.x < range->min.x || boundary->min.x >= range->max.x ||
		boundary->max.y < range->min.y || boundary->min.y >= range->max.y) {
		return false;
	};
	return true;
}

struct Vec2 aabb_get_center(struct AABB *boundary) {
	return (struct Vec2){
		.x = boundary->min.x + (boundary->max.x - boundary->min.x) / 2,
		.y = boundary->min.y + (boundary->max.y - boundary->min.y) / 2,
	};
}

void quadtree_free(struct QuadTree *qtree) {
	if (qtree->children[0]) {
		// the first child's pointer is the same pointer returned by malloc
		// for all the children's memory, so we only need to free that
		quadtree_free(qtree->children[0]);
	}
	free(qtree);
	qtree = NULL;
}

void quadtree_init(struct QuadTree *qtree, struct AABB *boundary, uint depth) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	qtree->boundary = *boundary;
	qtree->point_count = 0;
	qtree->depth = depth;
	qtree->children[0] = NULL;
	qtree_count++;
}

struct QuadTree *quadtree_new(struct AABB *boundary) {
	struct QuadTree *qtree = malloc(sizeof(*qtree));
	if (qtree == NULL) {
		return NULL;
	}
	quadtree_init(qtree, boundary, 0);
	return qtree;
}

uint quadtree_get_node_count() {
	return qtree_count;
}

uint quadtree_get_point_count() {
	return point_count;
}

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point) {
	if (!aabb_contains_point(&qtree->boundary, point)) {
		return false;
	}
	if (qtree->point_count < QT_NODE_CAPACITY && qtree->children[0] == NULL) {
		// we have room for more points and no children yet (assume if any children are NULL they all are)
		// so just add the point here and increment point count
		qtree->points[qtree->point_count++] = point;
		point_count++;
		return true;
	}
	if (qtree->children[0] == NULL) {
		// we don't have room for more points and need to subdivide
		struct QuadTree *children = malloc(sizeof(*children) * 4);
		if (children == NULL) {
			printf("ERROR: Failed to allocate children!\n");
			return false;
		}
		for (int i = 0; i < 4; ++i) {
			qtree->children[i] = &children[i];
		}
		struct Vec2 boundary_center = aabb_get_center(&qtree->boundary);
		quadtree_init(qtree->children[0], &(struct AABB){
			.min = qtree->boundary.min,
			.max = boundary_center,
		}, qtree->depth + 1);
		quadtree_init(qtree->children[1], &(struct AABB){
			.min = {.x = boundary_center.x, .y = qtree->boundary.min.y},
			.max = {.x = qtree->boundary.max.x, .y = boundary_center.y}
		}, qtree->depth + 1);
		quadtree_init(qtree->children[2], &(struct AABB){
			.min = {.x = qtree->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = qtree->boundary.max.y}
		}, qtree->depth + 1);
		quadtree_init(qtree->children[3], &(struct AABB){
			.min = boundary_center,
			.max = qtree->boundary.max,
		}, qtree->depth + 1);
	}
	// add new point to whichever child will accept it
	// it should always be accepted unless something weird has happened
	for (int i = 0; i < 4; ++i) {
		if (quadtree_add_point(qtree->children[i], point)) return true;
	}
	printf("ERROR: Unreachable code reached!\n");
	return false;
}

bool quadtree_point_in_range(struct QuadTree *qtree, struct AABB *range) {
	if (!aabb_intersects_range(&qtree->boundary, range)) {
		return false;
	}
	for (int i = 0; i < qtree->point_count; ++i) {
		if (aabb_contains_point(range, qtree->points[i])) return true;
	}
	if (qtree->children[0] == NULL) return false;
	for (int i = 0; i < 4; ++i) {
		if (quadtree_point_in_range(qtree->children[i], range)) return true;
	}
	return false;
}
