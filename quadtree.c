#include "quadtree.h"
#include <assert.h>
#include <stdio.h>

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
	if (qtree->child_nw) {
		// the first child's pointer is the same pointer returned by malloc
		// for all the children's memory, so we only need to free that
		quadtree_free(qtree->child_nw);
	}
	free(qtree);
	qtree = NULL;
}

void quadtree_init(struct QuadTree *qtree, struct AABB *boundary) {
	assert(boundary->min.x < boundary->max.x && boundary->min.y < boundary->max.y);
	qtree->boundary = *boundary;
	qtree->point_count = 0;
	qtree->depth = 0;
	qtree->child_nw = NULL;
	qtree_count++;
}

struct QuadTree *quadtree_new(struct AABB *boundary) {
	struct QuadTree *qtree = malloc(sizeof(*qtree));
	if (qtree == NULL) {
		return NULL;
	}
	quadtree_init(qtree, boundary);
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
	if (qtree->point_count < POINTS_PER_NODE && qtree->child_nw == NULL) {
		// we have room for more points and no children yet (assume if any children are NULL they all are)
		// so just add the point here and increment point count
		qtree->points[qtree->point_count++] = point;
		point_count++;
		return true;
	}
	if (qtree->child_nw == NULL) {
		// we don't have room for more points and need to subdivide
		struct QuadTree *children = malloc(sizeof(*children) * 4);
		if (children == NULL) {
			printf("ERROR: Failed to allocate children!\n");
			return false;
		}
		qtree->child_nw = &children[0];
		qtree->child_ne = &children[1];
		qtree->child_sw = &children[2];
		qtree->child_se = &children[3];

		struct Vec2 boundary_center = aabb_get_center(&qtree->boundary);

		quadtree_init(qtree->child_nw, &(struct AABB){
			.min = qtree->boundary.min,
			.max = boundary_center,
		});
		quadtree_init(qtree->child_ne, &(struct AABB){
			.min = {.x = boundary_center.x, .y = qtree->boundary.min.y},
			.max = {.x = qtree->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_init(qtree->child_sw, &(struct AABB){
			.min = {.x = qtree->boundary.min.x, .y = boundary_center.y},
			.max = {.x = boundary_center.x, .y = qtree->boundary.max.y}
		});
		quadtree_init(qtree->child_se, &(struct AABB){
			.min = boundary_center,
			.max = qtree->boundary.max,
		});
		qtree->child_nw->depth = qtree->depth + 1;
		qtree->child_ne->depth = qtree->depth + 1;
		qtree->child_sw->depth = qtree->depth + 1;
		qtree->child_se->depth = qtree->depth + 1;
	}
	// add new point to whichever child will accept it
	if (quadtree_add_point(qtree->child_nw, point)) return true;
	if (quadtree_add_point(qtree->child_ne, point)) return true;
	if (quadtree_add_point(qtree->child_sw, point)) return true;
	if (quadtree_add_point(qtree->child_se, point)) return true;

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
	if (qtree->child_nw == NULL) return false;
	if (quadtree_point_in_range(qtree->child_nw, range)) return true;
	if (quadtree_point_in_range(qtree->child_ne, range)) return true;
	if (quadtree_point_in_range(qtree->child_sw, range)) return true;
	if (quadtree_point_in_range(qtree->child_se, range)) return true;
	return false;
}
