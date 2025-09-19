#include <stdlib.h>
#include <stdio.h>

#include "quadtree.h"

#define BRUTEFORCE 0
#define QUADPOINTS 0
#define QUADCIRCLE 1

const uint ENTITY_COUNT = 100;
const uint WIDTH = 100;
const uint HEIGHT = 100;
const uint FRAMES = 1;
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

	srand(0);
	struct Vec2 points[ENTITY_COUNT];
	struct AABB ranges[ENTITY_COUNT];
	struct Circle circles[ENTITY_COUNT];
	int i, j;

	for (i = 0; i < ENTITY_COUNT; ++i) {
		points[i] = (struct Vec2){
			.x = (float)rand() / RAND_MAX * WIDTH,
			.y = (float)rand() / RAND_MAX * HEIGHT
		};
		ranges[i] = (struct AABB){
			.min = {.x = points[i].x, .y = points[i].y},
			.max = {.x = points[i].x + RANGE_SIZE, .y = points[i].y + RANGE_SIZE}
		};
		circles[i] = (struct Circle){
			.position = {.x = points[i].x, .y = points[i].y},
			.radius = (float)rand() / RAND_MAX * RANGE_SIZE + 1
		};
	}
	printf("entity count: %d\n", ENTITY_COUNT);

	struct timespec start_time;
	struct timespec end_time;
	struct timespec work_time;
	uint overlap_count = 0;

#if BRUTEFORCE
	// for every point check a range against every other point (SLOW!)
	overlap_count = 0;
	// clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (i = 0; i < ENTITY_COUNT; ++i) {
		for (j = 0; j < ENTITY_COUNT; ++j) {
			overlap_count += aabb_contains_point(&ranges[i], &points[j]);
		}
	}
	// clock_gettime(CLOCK_MONOTONIC, &end_time);
	// work_time = timespec_diff(&end_time, &start_time);
	// printf("bruteforce time: %f secs\n", timespec_to_secs(&work_time));
	printf("overlap count: %d\n", overlap_count);
#endif

#if QUADPOINTS
	// for every point use quadtree to check for overlap (FAST!)
	for (i = 0; i < FRAMES; ++i) {
		overlap_count = 0;
		// clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_clear(qtree);
		quadtree_add_points(qtree, points, ENTITY_COUNT);
		for (j = 0; j < ENTITY_COUNT; ++j) {
			overlap_count += quadtree_points_in_range(qtree, &ranges[j]);
		}
		// clock_gettime(CLOCK_MONOTONIC, &end_time);
		// work_time = timespec_diff(&end_time, &start_time);
		// printf("quadtree time: %f secs\n", timespec_to_secs(&work_time));
		printf("overlap count: %d\n", overlap_count);
	}
#endif

#if QUADCIRCLE
	// for every circle use quadtree to check for overlap (FAST!)
	struct Circle **overlapping_circles;
	for (i = 0; i < FRAMES; ++i) {
		overlap_count = 0;
		// clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_clear(qtree);
		quadtree_add_circles(qtree, circles, ENTITY_COUNT);
		printf("Added circles to quadtree\n");
		for (j = 0; j < ENTITY_COUNT; ++j) {
			printf("orig circle: x= %f, y= %f, r= %f\n", circles[j].position.x, circles[j].position.y, circles[j].radius);
			overlapping_circles = quadtree_circles_intersecting_circle(qtree, &circles[j]);
			while (overlapping_circles[overlap_count]) {
				printf("overlapping: x= %f, y= %f, r= %f\n", overlapping_circles[overlap_count]->position.x, overlapping_circles[overlap_count]->position.y, overlapping_circles[overlap_count]->radius);
				overlap_count++;
			}
			printf("overlapping circles: %d\n", overlap_count);
			overlap_count = 0;
			free(overlapping_circles);
		}
		// clock_gettime(CLOCK_MONOTONIC, &end_time);
		// work_time = timespec_diff(&end_time, &start_time);
		// printf("quadtree time: %f secs\n", timespec_to_secs(&work_time));
		// printf("overlap count: %d\n", overlap_count);
	}
#endif

	quadtree_free(qtree);
	return 0;
}
