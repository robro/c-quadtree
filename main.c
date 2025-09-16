#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "quadtree.h"

#define TOTAL_POINTS 10000
#define WIDTH 100
#define HEIGHT 100

int main(void) {
	struct QuadTree *qtree = quadtree_new(&(struct AABB){
		.min = {.x = 0, .y = 0},
		.max = {.x = WIDTH, .y = HEIGHT},
	});
	srand(time(NULL));
	struct Vec2 points[TOTAL_POINTS];
	for (int i = 0; i < TOTAL_POINTS; ++i) {
		points[i] = (struct Vec2){rand() % WIDTH, rand() % HEIGHT};
		quadtree_add_point(qtree, &points[i]);
	}
	printf("node count: %d\n", quadtree_get_node_count());
	printf("point count: %d\n", quadtree_get_point_count());

	printf("Range contains a point: %d\n", quadtree_point_in_range(qtree, &(struct AABB){
		.min = {.x = 0, .y = 0},
		.max = {.x = 1, .y = 1}
	}));
	return 0;
}
