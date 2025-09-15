#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_POINTS 4

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
	uint max_points;
	uint point_count;
	struct Vec2 **points;
	struct AABB aabb;

	struct QuadTree *top_left;
	struct QuadTree *top_right;
	struct QuadTree *bottom_left;
	struct QuadTree *bottom_right;
};

struct QuadTree *quadtree_new(struct AABB aabb) {
	struct QuadTree *qt = malloc(sizeof(*qt));
	if (qt == NULL) {
		return NULL;
	}
	struct Vec2 **points = malloc(sizeof(*points) * MAX_POINTS);
	if (points == NULL) {
		free(qt);
		return NULL;
	}
	qt->max_points = MAX_POINTS;
	qt->point_count = 0;
	qt->points = points;
	qt->aabb = aabb;
	qtree_count++;
	return qt;
}

bool quadtree_contains_point(struct QuadTree *qtree, struct Vec2 *point) {
	if (point->x < qtree->aabb.min.x || point->x >= qtree->aabb.max.x) {
		return false;
	}
	if (point->y < qtree->aabb.min.y || point->y >= qtree->aabb.max.y) {
		return false;
	}
	return true;
}

void quadtree_free(struct QuadTree *qtree) {
	if (qtree == NULL) {
		return;
	}
	if (qtree->top_left) quadtree_free(qtree->top_left);
	if (qtree->top_right) quadtree_free(qtree->top_right);
	if (qtree->bottom_left) quadtree_free(qtree->bottom_left);
	if (qtree->bottom_right) quadtree_free(qtree->bottom_right);
	free(qtree->points);
	free(qtree);
}

bool quadtree_add_point(struct QuadTree *qtree, struct Vec2 *point) {
	if (!quadtree_contains_point(qtree, point)) {
		return false;
	}
	if (qtree->point_count == qtree->max_points) {
		// if subquads don't exist, create them first
		if (qtree->top_left == NULL) {
			// assume if any subquad is NULL they all are
			float midpoint_x = qtree->aabb.min.x + (qtree->aabb.max.x - qtree->aabb.min.x) / 2;
			float midpoint_y = qtree->aabb.min.y + (qtree->aabb.max.y - qtree->aabb.min.y) / 2;

			qtree->top_left = quadtree_new((struct AABB){
				.min = qtree->aabb.min,
				.max = {.x = midpoint_x, .y = midpoint_y}
			});
			if (qtree->top_left == NULL) {
				goto free_subquads;
			}
			qtree->top_right = quadtree_new((struct AABB){
				.min = {.x = midpoint_x, .y = qtree->aabb.min.y},
				.max = {.x = qtree->aabb.max.x, .y = midpoint_y}
			});
			if (qtree->top_right == NULL) {
				goto free_subquads;
			}
			qtree->bottom_left = quadtree_new((struct AABB){
				.min = {.x = qtree->aabb.min.x, .y = midpoint_x},
				.max = {.x = midpoint_x, .y = qtree->aabb.max.y}
			});
			if (qtree->bottom_left == NULL) {
				goto free_subquads;
			}
			qtree->bottom_right = quadtree_new((struct AABB){
				.min = {.x = midpoint_x, .y = midpoint_x},
				.max = qtree->aabb.max,
			});
			if (qtree->bottom_right == NULL) {
				goto free_subquads;
			}
		}
		if (quadtree_add_point(qtree->top_left, point)) return true;
		if (quadtree_add_point(qtree->top_right, point)) return true;
		if (quadtree_add_point(qtree->bottom_left, point)) return true;
		if (quadtree_add_point(qtree->bottom_right, point)) return true;
		return false; // this should be unreachable

	free_subquads:
		quadtree_free(qtree->top_left);
		quadtree_free(qtree->top_right);
		quadtree_free(qtree->bottom_left);
		quadtree_free(qtree->bottom_right);
		return false;
	}
	qtree->points[qtree->point_count++] = point;
	return true;
}
