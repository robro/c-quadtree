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
	Vec2 velocity;
	Circle shape;
} EntityCircle;

typedef struct {
	uint size;
	uint capacity;
	void **array;
} DynamicArray;

Vec2 rect_get_center(Rect *rect);

bool rect_intersects_point(Rect *rect, Vec2 *point);

bool rect_intersects_rect(Rect *rect_1, Rect *rect_2);

bool rect_intersects_circle(Rect *rect, Circle *circle);

bool rect_intersects_entity_circle(Rect *rect, EntityCircle *entity_circle);

bool circle_intersects_circle(Circle *circle_1, Circle *circle_2);

bool entity_circle_intersects_entity_circle(EntityCircle *entity_circle_1, EntityCircle *entity_circle_2);

bool dynamic_array_init(void *array);

bool dynamic_array_push_back(void *array, void *value);

void dynamic_array_clear(void *array);

timespec timespec_diff(const timespec *time_1, const timespec *time_2);

float timespec_to_secs(const timespec *time);

#endif
