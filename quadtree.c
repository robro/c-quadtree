#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define POINTS_PER_NODE 4

uint qtree_count = 0;

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
	struct AABB boundary;
	struct Vec2 *points[POINTS_PER_NODE];

	struct QuadTree *north_west;
	struct QuadTree *north_east;
	struct QuadTree *south_west;
	struct QuadTree *south_east;
};

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point) {
	if (point->x < boundary->min.x || point->x >= boundary->max.x ||
		point->y < boundary->min.y || point->y >= boundary->max.y) {
		return false;
	}
	return true;
}

struct Vec2 aabb_get_center(struct AABB *boundary) {
	return (struct Vec2){
		.x = boundary->min.x + (boundary->max.x - boundary->min.x) / 2,
		.y = boundary->min.y + (boundary->max.y - boundary->min.y) / 2,
	};
}

void quadtree_init(struct QuadTree *qtree, struct AABB *boundary) {
	qtree->boundary = *boundary;
	qtree->point_count = 0;
}

void quadtree_free_children(struct QuadTree *qtree) {
	// the first child's pointer is the same pointer returned by malloc
	// for all the children's memory, so we only need to free that
	if (qtree->north_west) quadtree_free_children(qtree->north_west);
}

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point) {
	if (!aabb_contains_point(&qtree->boundary, point)) {
		return false;
	}
	if (qtree->point_count < POINTS_PER_NODE && qtree->north_west == NULL) {
		// we have room for more points and no children yet (assume if any children are NULL they all are)
		// so just add the point here and increment point count
		qtree->points[qtree->point_count++] = point;
		return true;
	}
	if (qtree->north_west == NULL) {
		// we don't have room for more points and need to subdivide
		struct QuadTree *children = malloc(sizeof(*children) * 4);
		if (children == NULL) {
			printf("quadtree_add_point(): Failed to allocate children!");
			return false;
		}
		qtree->north_west = &children[0];
		qtree->north_east = &children[1];
		qtree->south_west = &children[2];
		qtree->south_east = &children[3];

		struct Vec2 boundary_center = aabb_get_center(&qtree->boundary);

		quadtree_init(qtree->north_west, &(struct AABB){
			.min = qtree->boundary.min,
			.max = boundary_center,
		});
		quadtree_init(qtree->north_east, &(struct AABB){
			.min = {.x = boundary_center.x, .y = qtree->boundary.min.y},
			.max = {.x = qtree->boundary.max.x, .y = boundary_center.y}
		});
		quadtree_init(qtree->south_west, &(struct AABB){
			.min = {.x = qtree->boundary.min.x, .y = boundary_center.x},
			.max = {.x = boundary_center.x, .y = qtree->boundary.max.y}
		});
		quadtree_init(qtree->south_east, &(struct AABB){
			.min = boundary_center,
			.max = qtree->boundary.max,
		});

		// add existing points to children
		for (int i = 0; i < qtree->point_count; ++i) {
			if (quadtree_add_point(qtree->north_west, qtree->points[i])) continue;
			if (quadtree_add_point(qtree->north_east, qtree->points[i])) continue;
			if (quadtree_add_point(qtree->south_west, qtree->points[i])) continue;
			if (quadtree_add_point(qtree->south_east, qtree->points[i])) continue;
		}
	}
	// add new point to whichever child will accept it
	if (quadtree_add_point(qtree->north_west, point)) return true;
	if (quadtree_add_point(qtree->north_east, point)) return true;
	if (quadtree_add_point(qtree->south_west, point)) return true;
	if (quadtree_add_point(qtree->south_east, point)) return true;

	printf("quadtree_add_point(): Failed to add point due to error!");
	return false;
}
