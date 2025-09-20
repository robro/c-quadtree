#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#define NSECS_IN_SEC 1000000000

typedef struct timespec timespec;
typedef unsigned int uint;

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

Vec2 rect_get_center(Rect *rect);

bool rect_intersects_point(Rect *rect, Vec2 *point);

bool rect_intersects_rect(Rect *rect1, Rect *rect2);

bool rect_intersects_circle(Rect *rect, Circle *circle);

bool circle_intersects_circle(Circle *circle1, Circle *circle2);

bool point_array_init(PointArray *point_array);

bool point_array_push_back(PointArray *point_array, Vec2 *point);

void point_array_clear(PointArray *point_array);

bool circle_array_init(CircleArray *circle_array);

bool circle_array_push_back(CircleArray *circle_array, Circle *circle);

void circle_array_clear(CircleArray *circle_array);

bool rect_array_init(RectArray *rect_array);

bool rect_array_push_back(RectArray *rect_array, Rect *rect);

void rect_array_clear(RectArray *rect_array);

timespec timespec_diff(const timespec *time1, const timespec *time2);

float timespec_to_secs(const timespec *time);

#endif
