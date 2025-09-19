#include <stdlib.h>
#include <stdio.h>

#include "quadtree.h"
#include "util.h"

#define BRUTEFORCE 0
#define QUADRANGES 1
#define QUADCIRCLE 1

const uint ENTITY_COUNT = 1000;
const uint WIDTH = 100;
const uint HEIGHT = 100;
const uint FRAMES = 1;
const uint RADIUS = 5;

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
	int i, j, k;

	for (i = 0; i < ENTITY_COUNT; ++i) {
		points[i] = (struct Vec2){
			.x = (float)rand() / RAND_MAX * WIDTH,
			.y = (float)rand() / RAND_MAX * HEIGHT
		};
		ranges[i] = (struct AABB){
			.min = {.x = points[i].x - RADIUS, .y = points[i].y - RADIUS},
			.max = {.x = points[i].x + RADIUS, .y = points[i].y + RADIUS}
		};
		circles[i] = (struct Circle){
			.position = {.x = points[i].x, .y = points[i].y},
			.radius = RADIUS
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

#if QUADRANGES
	// for every point use quadtree to check for overlap (FAST!)
	struct AABBArray overlapping_ranges = {};
	range_array_init(&overlapping_ranges);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_ranges(qtree, ranges, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			quadtree_ranges_intersecting_range(qtree, &ranges[j], &overlapping_ranges);
			overlap_count += overlapping_ranges.size;
			range_array_clear(&overlapping_ranges);
		}
		quadtree_clear(qtree);
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		printf("range overlap count:  %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

#if QUADCIRCLE
	// for every circle use quadtree to check for overlap (FAST!)
	struct CircleArray overlapping_circles = {};
	circle_array_init(&overlapping_circles);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_circles(qtree, circles, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			// printf(
			// 	"orig circle: x= %f, y= %f, r= %f\n",
			// 	circles[j].position.x,
			// 	circles[j].position.y,
			// 	circles[j].radius
			// );
			quadtree_circles_intersecting_circle(qtree, &circles[j], &overlapping_circles);
			// for (k = 0; k < overlapping_circles.size; ++k) {
			// 	printf(
			// 		"overlapping: x= %f, y= %f, r= %f\n",
			// 		overlapping_circles.array[k]->position.x,
			// 		overlapping_circles.array[k]->position.y,
			// 		overlapping_circles.array[k]->radius
			// 	);
			// }
			// printf("overlapping circles: %d\n", overlapping_circles.size);
			overlap_count += overlapping_circles.size;
			circle_array_clear(&overlapping_circles);
		}
		quadtree_clear(qtree);
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		printf("circle overlap count: %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

	quadtree_free(qtree);
	return 0;
}
