#include <stdlib.h>
#include <stdio.h>

#include "quadtree.h"
#include "util.h"

#define QUADPOINTS 1
#define QUADRANGES 1
#define QUADCIRCLE 1

const uint ENTITY_COUNT = 10000;
const uint WIDTH = 100;
const uint HEIGHT = 100;
const uint FRAMES = 5;
const uint RADIUS = 5;

int main(void) {
	QuadTree *qtree = quadtree_new(&(Range2){
		.min = {.x = 0, .y = 0},
		.max = {.x = WIDTH, .y = HEIGHT},
	});
	if (qtree == NULL) {
		printf("ERROR: Failed to create quadtree!\n");
		return 1;
	}

	srand(0);
	Vec2 points[ENTITY_COUNT];
	Range2 ranges[ENTITY_COUNT];
	Circle circles[ENTITY_COUNT];
	int i, j, k;

	for (i = 0; i < ENTITY_COUNT; ++i) {
		points[i] = (Vec2){
			.x = (float)rand() / RAND_MAX * WIDTH,
			.y = (float)rand() / RAND_MAX * HEIGHT
		};
		ranges[i] = (Range2){
			.min = {.x = points[i].x - RADIUS, .y = points[i].y - RADIUS},
			.max = {.x = points[i].x + RADIUS, .y = points[i].y + RADIUS}
		};
		circles[i] = (Circle){
			.position = {.x = points[i].x, .y = points[i].y},
			.radius = RADIUS
		};
	}
	printf("entity count: %d\n", ENTITY_COUNT);

	timespec start_time;
	timespec end_time;
	timespec work_time;
	uint overlap_count;

#if QUADPOINTS
	// for every point check a range against every other point (SLOW!)
	PointArray overlapping_points = {};
	point_array_init(&overlapping_points);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_points(qtree, points, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			quadtree_points_in_range(qtree, &ranges[j], &overlapping_points);
			overlap_count += overlapping_points.size;
			point_array_clear(&overlapping_points);
		}
		quadtree_clear(qtree);
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		printf("point overlap count: %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

#if QUADRANGES
	// for every point use quadtree to check for overlap (FAST!)
	Range2Array overlapping_ranges = {};
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
		printf("range overlap count: %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

#if QUADCIRCLE
	// for every circle use quadtree to check for overlap (FAST!)
	CircleArray overlapping_circles = {};
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
