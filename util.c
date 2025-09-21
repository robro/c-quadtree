#include <math.h>
#include <stdlib.h>

#include "util.h"

#define ARRAY_DEFAULT_CAPACITY 2

struct timespec timespec_diff(const timespec *time_1, const timespec *time_2) {
	timespec diff = {
		.tv_sec = time_1->tv_sec - time_2->tv_sec,
		.tv_nsec = time_1->tv_nsec - time_2->tv_nsec,
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

static inline Vec2 vec2_add(const Vec2 *vec_1, const Vec2 *vec_2) {
	return (Vec2){
		.x = vec_1->x + vec_2->x,
		.y = vec_1->y + vec_2->y,
	};
}
static inline Vec2 vec2_diff(const Vec2 *vec_1, const Vec2 *vec_2) {
	return (Vec2){
		.x = vec_1->x - vec_2->x,
		.y = vec_1->y - vec_2->y,
	};
}

static inline float vec2_length(Vec2 *vec) {
	return sqrt(vec->x * vec->x + vec->y * vec->y);
}

Vec2 rect_get_center(Rect *rect) {
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

bool rect_intersects_circle(Rect *rect, Circle *circle) {
	Vec2 rect_center = rect_get_center(rect);
	Vec2 difference = vec2_diff(&circle->position, &rect_center);
	Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((rect->max.x - rect->min.x) / 2),
			 ((rect->max.x - rect->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((rect->max.y - rect->min.y) / 2),
			 ((rect->max.y - rect->min.y) / 2))
	};
	Vec2 closest = vec2_add(&rect_center, &clamped);
	difference = vec2_diff(&closest, &circle->position);
	return vec2_length(&difference) < circle->radius;
}

bool rect_intersects_rect(Rect *rect_1, Rect *rect_2) {
	if (rect_1->max.x < rect_2->min.x || rect_1->min.x >= rect_2->max.x ||
		rect_1->max.y < rect_2->min.y || rect_1->min.y >= rect_2->max.y) {
		return false;
	};
	return true;
}

bool rect_intersects_point(Rect *rect, Vec2 *point) {
	if (point->x < rect->min.x || point->x >= rect->max.x ||
		point->y < rect->min.y || point->y >= rect->max.y) {
		return false;
	}
	return true;
}

bool circle_intersects_circle(Circle *circle_1, Circle *circle_2) {
	Vec2 difference = vec2_diff(&circle_1->position, &circle_2->position);
	return vec2_length(&difference) < circle_1->radius + circle_2->radius;
}

bool rect_intersects_entity_circle(Rect *rect, EntityCircle *entity_circle) {
	return rect_intersects_circle(rect, &entity_circle->shape);
}

bool entity_circle_intersects_entity_circle(EntityCircle *entity_circle_1, EntityCircle *entity_circle_2) {
	return circle_intersects_circle(&entity_circle_1->shape, &entity_circle_2->shape);
}
