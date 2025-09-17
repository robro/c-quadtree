#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b) {
	struct timespec diff = {
		.tv_sec = time_a->tv_sec - time_b->tv_sec,
		.tv_nsec = time_a->tv_nsec - time_b->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const struct timespec *time) {
	return time->tv_sec + (float)time->tv_nsec / NSECS_IN_SEC;
}

void free_multiple(void **array, uint size) {
	for (int i = 0; i < size; ++i) {
		free(array[i]);
		array[i] = NULL;
	}
}

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point) {
	if (point->x < boundary->min.x || point->x >= boundary->max.x ||
		point->y < boundary->min.y || point->y >= boundary->max.y) {
		return false;
	}
	return true;
}

bool aabb_intersects_range(struct AABB *boundary, struct AABB *range) {
	if (boundary->max.x < range->min.x || boundary->min.x >= range->max.x ||
		boundary->max.y < range->min.y || boundary->min.y >= range->max.y) {
		return false;
	};
	return true;
}

struct Vec2 aabb_get_center(struct AABB *boundary) {
	return (struct Vec2){
		.x = boundary->min.x + (boundary->max.x - boundary->min.x) / 2,
		.y = boundary->min.y + (boundary->max.y - boundary->min.y) / 2,
	};
}

