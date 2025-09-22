#include <math.h>
#include <stdlib.h>

#include "util.h"

#define ARRAY_DEFAULT_CAPACITY 2

struct timespec timespec_subtract(const timespec *a, const timespec *b) {
	timespec diff = {
		.tv_sec = a->tv_sec - b->tv_sec,
		.tv_nsec = a->tv_nsec - b->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const timespec *time) {
	return time->tv_sec + (float)time->tv_nsec / NSECS_IN_SEC;
}

float clamp_float(float value, float min, float max) {
	const float v = (value < min) ? min : value;
	return (v > max) ? max : v;
}

Vec2 vec2_add(const Vec2 *a, const Vec2 *b) {
	return (Vec2){
		.x = a->x + b->x,
		.y = a->y + b->y,
	};
}

Vec2 vec2_subtract(const Vec2 *a, const Vec2 *b) {
	return (Vec2){
		.x = a->x - b->x,
		.y = a->y - b->y,
	};
}

Vec2 vec2_multiply(const Vec2 *vec, float multiplier) {
	return (Vec2){
		.x = vec->x * multiplier,
		.y = vec->y * multiplier,
	};
}

Vec2 vec2_divide(const Vec2 *vec, float divisor) {
	if (divisor == 0) {
		return *vec;
	}
	return (Vec2){
		.x = vec->x / divisor,
		.y = vec->y / divisor,
	};
}

float vec2_magnitude(const Vec2 *vec) {
	return sqrt(vec->x * vec->x + vec->y * vec->y);
}

Vec2 vec2_normalized(const Vec2 *vec) {
	return vec2_divide(vec, vec2_magnitude(vec));
}

float vec2_dot_product(const Vec2 *a, const Vec2 *b) {
	return a->x * b->x + a->y * b->y;
}

Vec2 vec2_direction(const Vec2 *from, const Vec2 *to) {
	const Vec2 difference = vec2_subtract(to, from);
	return vec2_normalized(&difference);
}

Vec2 rect_get_center(const Rect *rect) {
	return (Vec2){
		.x = rect->min.x + (rect->max.x - rect->min.x) / 2,
		.y = rect->min.y + (rect->max.y - rect->min.y) / 2,
	};
}

bool dynamic_array_init(void *array) {
	void **new_array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (new_array == NULL) {
		return false;
	}
	DynamicArray *dynamic_array = array;
	dynamic_array->size = 0;
	dynamic_array->capacity = ARRAY_DEFAULT_CAPACITY;
	dynamic_array->array = new_array;
	return true;
}

bool dynamic_array_push_back(void *array, void *value) {
	DynamicArray *dynamic_array = array;
	if (dynamic_array->size == dynamic_array->capacity) {
		void **new_array = realloc(dynamic_array->array, sizeof(void *) * dynamic_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		dynamic_array->array = new_array;
		dynamic_array->capacity *= 2;
	}
	dynamic_array->array[dynamic_array->size++] = value;
	return true;
}

void dynamic_array_clear(void *array) {
	DynamicArray *dynamic_array = array;
	dynamic_array->size = 0;
}

bool rect_intersects_circle(const Rect *rect, const Circle *circle) {
	Vec2 rect_center = rect_get_center(rect);
	Vec2 difference = vec2_subtract(&circle->position, &rect_center);
	Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((rect->max.x - rect->min.x) / 2),
			 ((rect->max.x - rect->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((rect->max.y - rect->min.y) / 2),
			 ((rect->max.y - rect->min.y) / 2))
	};
	Vec2 closest = vec2_add(&rect_center, &clamped);
	difference = vec2_subtract(&closest, &circle->position);
	return vec2_magnitude(&difference) < circle->radius;
}

bool rect_intersects_rect(const Rect *a, const Rect *b) {
	if (a->max.x < b->min.x || a->min.x >= b->max.x ||
		a->max.y < b->min.y || a->min.y >= b->max.y) {
		return false;
	};
	return true;
}

bool rect_intersects_point(const Rect *rect, const Vec2 *point) {
	if (point->x < rect->min.x || point->x >= rect->max.x ||
		point->y < rect->min.y || point->y >= rect->max.y) {
		return false;
	}
	return true;
}

bool circle_intersects_circle(const Circle *a, const Circle *b) {
	Vec2 difference = vec2_subtract(&a->position, &b->position);
	return vec2_magnitude(&difference) < a->radius + b->radius;
}

bool rect_intersects_entity_circle(const Rect *rect, const EntityCircle *circle) {
	return rect_intersects_circle(rect, &circle->shape);
}

bool entity_circle_intersects_entity_circle(const EntityCircle *a, const EntityCircle *b) {
	return circle_intersects_circle(&a->shape, &b->shape);
}
