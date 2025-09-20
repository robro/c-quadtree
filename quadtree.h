#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "util.h"

typedef struct QuadTree QuadTree;

QuadTree* quadtree_new(Range2 *boundary);

void quadtree_clear(QuadTree *qtree);

void quadtree_free(QuadTree *qtree);

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count);

void quadtree_add_ranges(QuadTree *qtree, Range2 *ranges, int range_count);

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count);

void quadtree_points_in_range(QuadTree *qtree, Range2 *range, PointArray *point_array);

void quadtree_ranges_intersecting_range(QuadTree *qtree, Range2 *range, Range2Array *range_array);

void quadtree_circles_intersecting_circle(QuadTree *qtree, Circle *circle, CircleArray *circle_array);

#endif
