#include <bits/time.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "quadtree.h"

#define TOTAL_POINTS 10000
#define WIDTH 100
#define HEIGHT 100
#define FRAMES 60

#define BENCH_BRUTEFORCE 0

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
	struct AABB range;
	int i, j;

	for (i = 0; i < TOTAL_POINTS; ++i) {
		points[i] = (struct Vec2){
			(float)rand() / RAND_MAX * WIDTH,
			(float)rand() / RAND_MAX * HEIGHT
		};
	}
	printf("point count: %d\n", TOTAL_POINTS);

	uint overlap_count = 0;
	struct timespec start_time;
	struct timespec end_time;
	struct timespec work_time;

#if BENCH_BRUTEFORCE
	// for every point check a range against every other point (SLOW!)
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (i = 0; i < TOTAL_POINTS; ++i) {
		range = (struct AABB){
			.min = {.x = points[i].x, points[i].y},
			.max = {.x = points[i].x + 5, .y = points[i].y + 5}
		};
		for (j = 0; j < TOTAL_POINTS; ++j) {
			overlap_count += aabb_contains_point(&range, &points[j]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	work_time = timespec_diff(&end_time, &start_time);
	printf("naive overlap check time: %f secs\n", timespec_to_secs(&work_time));
	overlap_count = 0;
#endif

	// for every point use quadtree to check for overlap (FAST!)
	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_clear(qtree);
		quadtree_add_points(qtree, points, TOTAL_POINTS);
		for (j = 0; j < TOTAL_POINTS; ++j) {
			range = (struct AABB){
				.min = {.x = points[j].x, points[j].y},
				.max = {.x = points[j].x + 5, .y = points[j].y + 5}
			};
			overlap_count += quadtree_points_in_range(qtree, &range);
		}
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		// printf("node capacity: %d points\n", QT_NODE_CAPACITY);
		// printf("node count: %d\n", qtree->node_count);
		printf("qtree time: %f secs\n", timespec_to_secs(&work_time));
		overlap_count = 0;
	}

	return 0;
}
