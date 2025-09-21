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

Vec2 vec2_add(const Vec2 *vec_1, const Vec2 *vec_2);

Vec2 vec2_diff(const Vec2 *vec_1, const Vec2 *vec_2);

Vec2 vec2_mult(const Vec2 *vec, float num);

Vec2 vec2_div(const Vec2 *vec, float num);

Vec2 vec2_normalized(const Vec2 *vec);

Vec2 vec2_direction(const Vec2 *vec_from, const Vec2 *vec_to);

float vec2_length(const Vec2 *vec);

Vec2 rect_get_center(const Rect *rect);

bool rect_intersects_point(const Rect *rect, const Vec2 *point);

bool rect_intersects_rect(const Rect *rect_1, const Rect *rect_2);

bool rect_intersects_circle(const Rect *rect, const Circle *circle);

bool rect_intersects_entity_circle(const Rect *rect, const EntityCircle *entity_circle);

bool circle_intersects_circle(const Circle *circle_1, const Circle *circle_2);

bool entity_circle_intersects_entity_circle(const EntityCircle *entity_circle_1, const EntityCircle *entity_circle_2);

bool dynamic_array_init(void *array);

bool dynamic_array_push_back(void *array, void *value);

void dynamic_array_clear(void *array);

timespec timespec_diff(const timespec *time_1, const timespec *time_2);

float timespec_to_secs(const timespec *time);

#endif
