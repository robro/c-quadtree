#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define QT_NODE_CAPACITY 4

struct Vec2 {
	float x;
	float y;
};

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

struct QuadTree *quadtree_new(struct AABB *boundary);

void quadtree_free(struct QuadTree *qtree);

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point);

bool quadtree_point_in_range(struct QuadTree *qtree, struct AABB *range);

uint quadtree_get_node_count();

uint quadtree_get_point_count();

#endif
