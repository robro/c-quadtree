#ifndef QUADTREE_H
#define QUADTREE_H

#include "util.h"

typedef struct QuadTree QuadTree;

QuadTree *quadtree_new(Rect *boundary);

void quadtree_clear(QuadTree *qtree);

void quadtree_free(QuadTree *qtree);

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count);

void quadtree_add_rects(QuadTree *qtree, Rect *rects, int rect_count);

void quadtree_add_circle(QuadTree *qtree, Circle *circle);

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count);

void quadtree_add_entities_circle(QuadTree *qtree, EntityCircle *entities_circle, int count);

void quadtree_points_intersecting_rect(QuadTree *qtree, Rect *rect, DynamicArray *results);

void quadtree_rects_intersecting_rect(QuadTree *qtree, Rect *rect, DynamicArray *results);

void quadtree_circles_intersecting_circle(QuadTree *qtree, Circle *circle, DynamicArray *results);

void quadtree_entities_circle_intersecting_entity_circle(QuadTree *qtree, EntityCircle *entity_circle, DynamicArray *results);

#endif
