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

#define VEC2_ZERO (Vec2){0, 0}

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

Vec2 vec2_add(const Vec2 *a, const Vec2 *b);

Vec2 vec2_subtract(const Vec2 *a, const Vec2 *b);

Vec2 vec2_multiply(const Vec2 *vec, float multiplier);

Vec2 vec2_divide(const Vec2 *vec, float divisor);

Vec2 vec2_normalized(const Vec2 *vec);

Vec2 vec2_direction(const Vec2 *from, const Vec2 *to);

float vec2_dot_product(const Vec2 *a, const Vec2 *b);

float vec2_magnitude(const Vec2 *vec);

Vec2 rect_get_center(const Rect *rect);

bool rect_intersects_point(const Rect *rect, const Vec2 *point);

bool rect_intersects_rect(const Rect *a, const Rect *b);

bool rect_intersects_circle(const Rect *rect, const Circle *circle);

bool rect_intersects_entity_circle(const Rect *rect, const EntityCircle *circle);

bool circle_intersects_circle(const Circle *a, const Circle *b);

bool entity_circle_intersects_entity_circle(const EntityCircle *a, const EntityCircle *b);

bool dynamic_array_init(void *array);

bool dynamic_array_push_back(void *array, void *value);

void dynamic_array_clear(void *array);

timespec timespec_subtract(const timespec *a, const timespec *b);

float timespec_to_secs(const timespec *time);

#endif
