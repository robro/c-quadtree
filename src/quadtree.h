#ifndef QUADTREE_H
#define QUADTREE_H

#include "util.h"

typedef struct QuadTree QuadTree;

QuadTree *quadtree_new(const Rect *boundary);

void quadtree_clear(QuadTree *qtree);

void quadtree_free(QuadTree *qtree);

uint quadtree_get_size(QuadTree *qtree);

void quadtree_add_points(QuadTree *qtree, Vec2 *points, int point_count);

void quadtree_add_rects(QuadTree *qtree, Rect *rects, int rect_count);

void quadtree_add_circles(QuadTree *qtree, Circle *circles, int circle_count);

uint quadtree_add_entities_circle(QuadTree *qtree, EntityCircle *entities_circle, int count);

void quadtree_points_intersecting_rect(const QuadTree *qtree, const Rect *rect, DynamicArray *results);

void quadtree_rects_intersecting_rect(const QuadTree *qtree, const Rect *rect, DynamicArray *results);

void quadtree_circles_intersecting_circle(const QuadTree *qtree, const Circle *circle, DynamicArray *results);

void quadtree_entities_circle_intersecting_entity_circle(const QuadTree *qtree, const EntityCircle *entity_circle, DynamicArray *results);

#endif
