#ifndef QUADTREE_H
#define QUADTREE_H

#include "util.h"

typedef struct QuadTree QuadTree;

QuadTree *quadtree_new(Rect *boundary);

void quadtree_clear(QuadTree *qtree);

void quadtree_free(QuadTree *qtree);

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count);

void quadtree_add_rects(QuadTree *qtree, Rect *rects, int rect_count);

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count);

void quadtree_points_intersecting_rect(QuadTree *qtree, Rect *rect, PointArray *point_array);

void quadtree_rects_intersecting_rect(QuadTree *qtree, Rect *rect, RectArray *rect_array);

void quadtree_circles_intersecting_circle(QuadTree *qtree, Circle *circle, CircleArray *circle_array);

#endif
