#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "util.h"

#define QT_NODE_CAPACITY 4

struct AABB {
	struct Vec2 min;
	struct Vec2 max;
};

struct QuadTree {
	uint depth;
	uint point_count;
	struct AABB boundary;
	struct Vec2 *points[QT_NODE_CAPACITY];
	struct QuadTree *children[4];
};

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point);

struct QuadTree *quadtree_new(struct AABB *boundary);

void quadtree_free(struct QuadTree *qtree);

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point);

uint quadtree_points_in_range(struct QuadTree *qtree, struct AABB *range);

uint quadtree_get_node_count();

uint quadtree_get_point_count();

#endif
