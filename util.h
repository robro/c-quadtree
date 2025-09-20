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
} Rect;

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
	Rect **array;
} RectArray;

typedef struct {
	uint size;
	uint capacity;
	Circle **array;
} CircleArray;

#define VEC_ZERO (Vec2){0, 0}

Vec2 rect_get_center(Rect *rect);

bool rect_intersects_point(Rect *rect, Vec2 *point);

bool rect_intersects_rect(Rect *r1, Rect *r2);

bool rect_intersects_circle(Rect *rect, Circle *circle);

bool circle_intersects_circle(Circle *c1, Circle *c2);

bool point_array_init(PointArray *point_array);

bool point_array_push_back(PointArray *point_array, Vec2 *point);

void point_array_clear(PointArray *point_array);

bool circle_array_init(CircleArray *circle_array);

bool circle_array_push_back(CircleArray *circle_array, Circle *circle);

void circle_array_clear(CircleArray *circle_array);

bool rect_array_init(RectArray *rect_array);

bool rect_array_push_back(RectArray *rect_array, Rect *rect);

void rect_array_clear(RectArray *rect_array);

timespec timespec_diff(const timespec *t1, const timespec *t2);

float timespec_to_secs(const timespec *time);

void free_multiple(void **array, uint size);

#endif
