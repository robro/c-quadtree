#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "util.h"

#define ARRAY_DEFAULT_CAPACITY 2

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b) {
	struct timespec diff = {
		.tv_sec = time_a->tv_sec - time_b->tv_sec,
		.tv_nsec = time_a->tv_nsec - time_b->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const struct timespec *time) {
	return time->tv_sec + (float)time->tv_nsec / NSECS_IN_SEC;
}

void free_multiple(void **array, uint size) {
	for (int i = 0; i < size; ++i) {
		free(array[i]);
		array[i] = NULL;
	}
}

bool aabb_contains_point(struct AABB *boundary, struct Vec2 *point) {
	if (point->x < boundary->min.x || point->x >= boundary->max.x ||
		point->y < boundary->min.y || point->y >= boundary->max.y) {
		return false;
	}
	return true;
}

float clamp_float(float value, float min, float max) {
	const float v = (value < min) ? min : value;
	return (v > max) ? max : v;
}

float vec2_length(struct Vec2 *v) {
	return sqrt(v->x * v->x + v->y * v->y);
}

bool circle_intersects_circle(struct Circle *circle_1, struct Circle *circle_2) {
	struct Vec2 difference = {
		.x = circle_1->position.x - circle_2->position.x,
		.y = circle_1->position.y - circle_2->position.y,
	};
	return vec2_length(&difference) < circle_1->radius + circle_2->radius;
}

bool range_array_init(struct AABBArray *range_array) {
	struct AABB **array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	range_array->size = 0;
	range_array->capacity = ARRAY_DEFAULT_CAPACITY;
	range_array->array = array;
	return true;
}

bool range_array_push_back(struct AABBArray *range_array, struct AABB *range) {
	if (range_array->size == range_array->capacity) {
		struct AABB **new_array = realloc(range_array->array, sizeof(struct Circle *) * range_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		range_array->array = new_array;
		range_array->capacity *= 2;
	}
	range_array->array[range_array->size++] = range;
	return true;
}

void range_array_clear(struct AABBArray *range_array) {
	range_array->size = 0;
}

bool circle_array_init(struct CircleArray *circle_array) {
	struct Circle **array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	circle_array->size = 0;
	circle_array->capacity = ARRAY_DEFAULT_CAPACITY;
	circle_array->array = array;
	return true;
}

bool circle_array_push_back(struct CircleArray *circle_array, struct Circle *circle) {
	if (circle_array->size == circle_array->capacity) {
		struct Circle **new_array = realloc(circle_array->array, sizeof(struct Circle *) * circle_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		circle_array->array = new_array;
		circle_array->capacity *= 2;
	}
	circle_array->array[circle_array->size++] = circle;
	return true;
}

void circle_array_clear(struct CircleArray *circle_array) {
	circle_array->size = 0;
}

bool aabb_intersects_circle(struct AABB *boundary, struct Circle *circle) {
	struct Vec2 boundary_center = aabb_get_center(boundary);
	struct Vec2 difference = {
		.x = circle->position.x - boundary_center.x,
		.y = circle->position.y - boundary_center.y
	};
	struct Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((boundary->max.x - boundary->min.x) / 2),
			 ((boundary->max.x - boundary->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((boundary->max.y - boundary->min.y) / 2),
			 ((boundary->max.y - boundary->min.y) / 2))
	};
	struct Vec2 closest = {
		.x = boundary_center.x + clamped.x,
		.y = boundary_center.y + clamped.y
	};
	difference = (struct Vec2){
		.x = closest.x - circle->position.x,
		.y = closest.y - circle->position.y
	};
	return vec2_length(&difference) < circle->radius;
}

bool aabb_intersects_range(struct AABB *boundary, struct AABB *range) {
	if (boundary->max.x < range->min.x || boundary->min.x >= range->max.x ||
		boundary->max.y < range->min.y || boundary->min.y >= range->max.y) {
		return false;
	};
	return true;
}

struct Vec2 aabb_get_center(struct AABB *boundary) {
	return (struct Vec2){
		.x = boundary->min.x + (boundary->max.x - boundary->min.x) / 2,
		.y = boundary->min.y + (boundary->max.y - boundary->min.y) / 2,
	};
}

