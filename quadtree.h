#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "util.h"

struct QuadTree;

struct QuadTree* quadtree_new(struct AABB *boundary);

void quadtree_clear(struct QuadTree *qtree);

void quadtree_free(struct QuadTree *qtree);

void quadtree_add_points(struct QuadTree *qtree, struct Vec2 *points, int point_count);

void quadtree_add_circles(struct QuadTree *qtree, struct Circle *circles, int circle_count);

uint quadtree_points_in_range(struct QuadTree *qtree, struct AABB *range);

struct Circle **quadtree_circles_intersecting_circle(struct QuadTree *qtree, struct Circle *circle);

#endif
