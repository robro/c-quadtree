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

struct Circle {
	float radius;
	struct Vec2 position;
};

struct CircleArray {
	uint size;
	uint capacity;
	struct Circle **array;
};

#define VEC_ZERO (struct Vec2){0, 0}

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point);

bool aabb_intersects_range(struct AABB *boundary, struct AABB *range);

bool aabb_intersects_circle(struct AABB *boundary, struct Circle *circle);

struct Vec2 aabb_get_center(struct AABB *boundary);

bool circle_intersects_circle(struct Circle *circle_1, struct Circle *circle_2);

bool circle_array_init(struct CircleArray *circle_array);

bool circle_array_push_back(struct CircleArray *circle_array, struct Circle *circle);

void circle_array_clear(struct CircleArray *circle_array);

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b);

float timespec_to_secs(const struct timespec *time);

void free_multiple(void **array, uint size);

#endif
