#ifndef QUADTREE_H
#define QUADTREE_H

#include "util.h"

typedef struct QuadTree QuadTree;

QuadTree *quadtree_new(const AABB *boundary);

void quadtree_clear(QuadTree *qtree);

void quadtree_free(QuadTree *qtree);

uint quadtree_get_size(QuadTree *qtree);

uint quadtree_add_entities_rect(QuadTree *qtree, Entity *rects, int count);

uint quadtree_add_entities_circle(QuadTree *qtree, Entity *circles, int count);

void quadtree_rects_intersecting_rect(const QuadTree *qtree, const Rect *rect, DynamicArray *results);

void quadtree_circles_intersecting_circle(const QuadTree *qtree, const Circle *circle, DynamicArray *results);

void quadtree_entities_circle_intersecting_entity_circle(const QuadTree *qtree, const Entity *circle, DynamicArray *results);

void quadtree_entities_rect_intersecting_entity_rect(const QuadTree *qtree, const Entity *rect, DynamicArray *results);

#endif
