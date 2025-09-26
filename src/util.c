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

Vec2 aabb_get_center(const AABB *aabb) {
	return (Vec2){
		.x = aabb->min.x + (aabb->max.x - aabb->min.x) / 2,
		.y = aabb->min.y + (aabb->max.y - aabb->min.y) / 2,
	};
}

AABB aabb_get_from_entity_rect(const EntityRect *rect) {
	return (AABB){
		.min = {
			.x = rect->base.position.x - rect->shape.width / 2,
			.y = rect->base.position.y - rect->shape.height / 2,
		},
		.max = {
			.x = rect->base.position.x + rect->shape.width / 2,
			.y = rect->base.position.y + rect->shape.height / 2,
		},
	};
}

bool dynamic_array_init(DynamicArray *array) {
	void **new_array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (new_array == NULL) {
		return false;
	}
	array->size = 0;
	array->capacity = ARRAY_DEFAULT_CAPACITY;
	array->array = new_array;
	return true;
}

bool dynamic_array_push_back(DynamicArray *array, void *value) {
	if (array->size == array->capacity) {
		void **new_array = realloc(array->array, sizeof(void *) * array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		array->array = new_array;
		array->capacity *= 2;
	}
	array->array[array->size++] = value;
	return true;
}

void dynamic_array_clear(DynamicArray *array) {
	array->size = 0;
}

bool aabb_intersects_entity_circle(const AABB *aabb, const EntityCircle *circle) {
	Vec2 aabb_center = aabb_get_center(aabb);
	Vec2 difference = vec2_subtract(&circle->base.position, &aabb_center);
	Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((aabb->max.x - aabb->min.x) / 2),
			 ((aabb->max.x - aabb->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((aabb->max.y - aabb->min.y) / 2),
			 ((aabb->max.y - aabb->min.y) / 2))
	};
	Vec2 closest = vec2_add(&aabb_center, &clamped);
	difference = vec2_subtract(&closest, &circle->base.position);
	return vec2_magnitude(&difference) < circle->shape.radius;
}

bool aabb_intersects_aabb(const AABB *a, const AABB *b) {
	return (a->max.x > b->min.x && a->min.x < b->max.x &&
			a->max.y > b->min.y && a->min.y < b->max.y);
}

bool aabb_intersects_entity_rect(const AABB *aabb, const EntityRect *entity_rect) {
	AABB aabb_b = aabb_get_from_entity_rect(entity_rect);
	return aabb_intersects_aabb(aabb, &aabb_b);
}

bool entity_circle_intersects_entity_circle(const EntityCircle *a, const EntityCircle *b) {
	Vec2 difference = vec2_subtract(&a->base.position, &b->base.position);
	return vec2_magnitude(&difference) < a->shape.radius + b->shape.radius;
}

bool entity_rect_intersects_entity_rect(const EntityRect *a, const EntityRect *b) {
	AABB aabb_a = aabb_get_from_entity_rect(a);
	AABB aabb_b = aabb_get_from_entity_rect(b);
	return aabb_intersects_aabb(&aabb_a, &aabb_b);
}
