#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "util.h"

#define QT_MAX_NODES 2000
#define QT_NODE_CAPACITY 10

struct QuadTreeNode {
	uint depth;
	uint point_count;
	struct AABB boundary;
	struct Vec2 *points[QT_NODE_CAPACITY];
	struct QuadTreeNode *children[4];
};

struct QuadTree {
	struct AABB boundary;
	uint node_count;
	struct QuadTreeNode nodes[QT_MAX_NODES];
};

struct QuadTree *quadtree_new(struct AABB *boundary);

void quadtree_clear(struct QuadTree *qtree);

void quadtree_add_points(struct QuadTree *qtree, struct Vec2 *points, uint point_count);

uint quadtree_points_in_range(struct QuadTree *qtree, struct AABB *range);

#endif
