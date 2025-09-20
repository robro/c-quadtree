#include <stdlib.h>
#include <stdio.h>

#include "quadtree.h"
#include "util.h"

#define PNTS_BENCH 1
#define RECT_BENCH 1
#define CIRC_BENCH 1

const uint ENTITY_COUNT = 10000;
const uint WIDTH = 100;
const uint HEIGHT = 100;
const uint FRAMES = 5;
const float RADIUS = 2.5;

int main(void) {
	QuadTree *qtree = quadtree_new(&(Rect){
		.min = {.x = 0, .y = 0},
		.max = {.x = WIDTH, .y = HEIGHT},
	});
	if (qtree == NULL) {
		printf("ERROR: Failed to create quadtree!\n");
		return 1;
	}

	srand(0);
	Vec2 points[ENTITY_COUNT];
	Rect rects[ENTITY_COUNT];
	Circle circles[ENTITY_COUNT];
	int i, j, k;

	for (i = 0; i < ENTITY_COUNT; ++i) {
		points[i] = (Vec2){
			.x = (float)rand() / RAND_MAX * WIDTH,
			.y = (float)rand() / RAND_MAX * HEIGHT
		};
		rects[i] = (Rect){
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

#if PNTS_BENCH
	PointArray overlapping_points = {};
	point_array_init(&overlapping_points);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_points(qtree, points, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			quadtree_points_intersecting_rect(qtree, &rects[j], &overlapping_points);
			overlap_count += overlapping_points.size;
			point_array_clear(&overlapping_points);
		}
		quadtree_clear(qtree);
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		printf("point overlap count: %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

#if RECT_BENCH
	RectArray overlapping_rects = {};
	rect_array_init(&overlapping_rects);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_rects(qtree, rects, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			quadtree_rects_intersecting_rect(qtree, &rects[j], &overlapping_rects);
			overlap_count += overlapping_rects.size;
			rect_array_clear(&overlapping_rects);
		}
		quadtree_clear(qtree);
		clock_gettime(CLOCK_MONOTONIC, &end_time);
		work_time = timespec_diff(&end_time, &start_time);
		printf("range overlap count: %d | time: %f secs\n", overlap_count, timespec_to_secs(&work_time));
	}
#endif

#if CIRC_BENCH
	CircleArray overlapping_circles = {};
	circle_array_init(&overlapping_circles);

	for (i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		quadtree_add_circles(qtree, circles, ENTITY_COUNT);
		overlap_count = 0;
		for (j = 0; j < ENTITY_COUNT; ++j) {
			quadtree_circles_intersecting_circle(qtree, &circles[j], &overlapping_circles);
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
