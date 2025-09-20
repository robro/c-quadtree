#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <time.h>

#define NSECS_IN_SEC 1000000000

#define array_size(array) (sizeof(array) / sizeof(*array))

typedef struct timespec timespec;

typedef struct {
	float x;
	float y;
} Vec2;

typedef struct {
	Vec2 min;
	Vec2 max;
} Range2;

typedef struct {
	float radius;
	Vec2 position;
} Circle;

typedef struct {
	uint size;
	uint capacity;
	Vec2 **array;
} PointArray;

typedef struct {
	uint size;
	uint capacity;
	Range2 **array;
} Range2Array;

typedef struct {
	uint size;
	uint capacity;
	Circle **array;
} CircleArray;

#define VEC_ZERO (Vec2){0, 0}

Vec2 range_get_center(Range2 *range);

bool range_contains_point(Range2 *range, Vec2 *point);

bool range_intersects_range(Range2 *r1, Range2 *r2);

bool range_intersects_circle(Range2 *range, Circle *circle);

bool circle_intersects_circle(Circle *circle_a, Circle *circle_b);

bool point_array_init(PointArray *point_array);

bool point_array_push_back(PointArray *point_array, Vec2 *point);

void point_array_clear(PointArray *point_array);

bool circle_array_init(CircleArray *circle_array);

bool circle_array_push_back(CircleArray *circle_array, Circle *circle);

void circle_array_clear(CircleArray *circle_array);

bool range_array_init(Range2Array *range_array);

bool range_array_push_back(Range2Array *range_array, Range2 *range);

void range_array_clear(Range2Array *range_array);

timespec timespec_diff(const timespec *t1, const timespec *t2);

float timespec_to_secs(const timespec *time);

void free_multiple(void **array, uint size);

#endif
