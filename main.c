#include "quadtree.h"
#include <stdio.h>

#define BRUTEFORCE 0

const uint TOTAL_POINTS = 10000;
const uint WIDTH = 100;
const uint HEIGHT = 100;
const uint FRAMES = 10;
const uint RANGE_SIZE = 5;

int main(void) {
	struct QuadTree *qtree = quadtree_new(&(struct AABB){
		.min = {.x = 0, .y = 0},
		.max = {.x = WIDTH, .y = HEIGHT},
	});
	if (qtree == NULL) {
		printf("ERROR: Failed to create quadtree!\n");
		return 1;
	}

	srand(time(NULL));
	struct Vec2 points[TOTAL_POINTS];
	struct AABB ranges[TOTAL_POINTS];
	int i, j;

	for (i = 0; i < TOTAL_POINTS; ++i) {
		points[i] = (struct Vec2){
			.x = (float)rand() / RAND_MAX * WIDTH,
			.y = (float)rand() / RAND_MAX * HEIGHT
		};
		ranges[i] = (struct AABB){
			.min = {.x = points[j].x, .y = points[j].y},
			.max = {.x = points[j].x + RANGE_SIZE, .y = points[j].y + RANGE_SIZE}
		};
	}
	printf("point count: %d\n", TOTAL_POINTS);

	struct timespec start_time;
	struct timespec end_time;
	struct timespec work_time;

#if BRUTEFORCE
	// for every point check a range against every other point (SLOW!)
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (i = 0; i < TOTAL_POINTS; ++i) {
		for (j = 0; j < TOTAL_POINTS; ++j) {
			aabb_contains_point(&ranges[i], &points[j]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	work_time = timespec_diff(&end_time, &start_time);
	printf("bruteforce time: %f secs\n", timespec_to_secs(&work_time));
#endif

	// for every point use quadtree to check for overlap (FAST!)
	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_clear(qtree);
		quadtree_add_points(qtree, points, TOTAL_POINTS);
		for (j = 0; j < TOTAL_POINTS; ++j) {
			quadtree_points_in_range(qtree, &ranges[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		// printf("node capacity: %d points\n", QT_NODE_CAPACITY);
		// printf("node count: %d\n", qtree->node_count);
		printf("quadtree time: %f secs\n", timespec_to_secs(&work_time));
	}

	quadtree_free(qtree);
	return 0;
}
