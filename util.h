#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <time.h>

#define NSECS_IN_SEC 1000000000

#define array_size(array) (sizeof(array) / sizeof(*array))

struct Vec2 {
	float x;
	float y;
};

struct AABB {
	struct Vec2 min;
	struct Vec2 max;
};

#define VEC_ZERO (struct Vec2){0, 0}

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point);

bool aabb_intersects_range(struct AABB *boundary, struct AABB *range);

struct Vec2 aabb_get_center(struct AABB *boundary);

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b);

float timespec_to_secs(const struct timespec *time);

void free_multiple(void **array, uint size);

#endif
