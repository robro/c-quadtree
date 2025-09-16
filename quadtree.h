#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define POINTS_PER_NODE 4

struct Vec2 {
	float x;
	float y;
};

struct AABB {
	struct Vec2 min;
	struct Vec2 max;
};

struct QuadTree {
	uint point_count;
	uint depth;
	struct AABB boundary;
	struct Vec2 *points[POINTS_PER_NODE];

	struct QuadTree *child_nw;
	struct QuadTree *child_ne;
	struct QuadTree *child_sw;
	struct QuadTree *child_se;
};

struct QuadTree *quadtree_new(struct AABB *boundary);

void quadtree_free(struct QuadTree *qtree);

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point);

bool quadtree_point_in_range(struct QuadTree *qtree, struct AABB *range);

uint quadtree_get_node_count();

uint quadtree_get_point_count();

#endif
