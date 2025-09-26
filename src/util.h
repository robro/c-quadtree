#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#define NSECS_IN_SEC 1000000000

typedef struct timespec timespec;
typedef unsigned int uint;

typedef struct Vec2 {
	float x;
	float y;
} Vec2;

#define VEC2_ZERO (Vec2){0, 0}

typedef struct AABB {
	Vec2 min;
	Vec2 max;
} AABB;

typedef struct Rect {
	float width;
	float height;
} Rect;

typedef struct Circle {
	float radius;
} Circle;

typedef struct Entity {
	Vec2 position;
	Vec2 velocity;
	union {
		Rect rect;
		Circle circle;
	} shape;
} Entity;

typedef struct DynamicArray {
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

float vec2_magnitude_squared(const Vec2 *vec);

Vec2 aabb_get_center(const AABB *rect);

bool aabb_intersects_entity_rect(const AABB *aabb, const Entity *rect);

bool aabb_intersects_entity_circle(const AABB *aabb, const Entity *rect);

bool entity_circle_intersects_entity_circle(const Entity *a, const Entity *b);

bool entity_rect_intersects_entity_rect(const Entity *a, const Entity *b);

bool dynamic_array_init(DynamicArray *array);

bool dynamic_array_push_back(DynamicArray *array, void *value);

void dynamic_array_clear(DynamicArray *array);

void dynamic_array_free(DynamicArray *array);

timespec timespec_subtract(const timespec *a, const timespec *b);

float timespec_to_secs(const timespec *time);

#endif
