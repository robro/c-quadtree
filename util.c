#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "util.h"

#define ARRAY_DEFAULT_CAPACITY 2

struct timespec timespec_diff(const timespec *t1, const timespec *t2) {
	timespec diff = {
		.tv_sec = t1->tv_sec - t2->tv_sec,
		.tv_nsec = t1->tv_nsec - t2->tv_nsec,
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

void free_multiple(void **array, uint size) {
	for (int i = 0; i < size; ++i) {
		free(array[i]);
		array[i] = NULL;
	}
}

float clamp_float(float value, float min, float max) {
	const float v = (value < min) ? min : value;
	return (v > max) ? max : v;
}

float vec2_length(Vec2 *v) {
	return sqrt(v->x * v->x + v->y * v->y);
}

bool range_contains_point(Range2 *range, Vec2 *point) {
	if (point->x < range->min.x || point->x >= range->max.x ||
		point->y < range->min.y || point->y >= range->max.y) {
		return false;
	}
	return true;
}

bool circle_intersects_circle(Circle *c1, Circle *c2) {
	Vec2 difference = {
		.x = c1->position.x - c2->position.x,
		.y = c1->position.y - c2->position.y,
	};
	return vec2_length(&difference) < c1->radius + c2->radius;
}

bool range_array_init(Range2Array *range_array) {
	Range2 **array = malloc(sizeof(*array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	range_array->size = 0;
	range_array->capacity = ARRAY_DEFAULT_CAPACITY;
	range_array->array = array;
	return true;
}

bool range_array_push_back(Range2Array *range_array, Range2 *range) {
	if (range_array->size == range_array->capacity) {
		Range2 **new_array = realloc(range_array->array, sizeof(*new_array) * range_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		range_array->array = new_array;
		range_array->capacity *= 2;
	}
	range_array->array[range_array->size++] = range;
	return true;
}

void range_array_clear(Range2Array *range_array) {
	range_array->size = 0;
}

bool circle_array_init(CircleArray *circle_array) {
	Circle **array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	circle_array->size = 0;
	circle_array->capacity = ARRAY_DEFAULT_CAPACITY;
	circle_array->array = array;
	return true;
}

bool circle_array_push_back(CircleArray *circle_array, Circle *circle) {
	if (circle_array->size == circle_array->capacity) {
		Circle **new_array = realloc(circle_array->array, sizeof(*new_array) * circle_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		circle_array->array = new_array;
		circle_array->capacity *= 2;
	}
	circle_array->array[circle_array->size++] = circle;
	return true;
}

void circle_array_clear(CircleArray *circle_array) {
	circle_array->size = 0;
}

bool range_intersects_circle(Range2 *range, Circle *circle) {
	Vec2 range_center = range_get_center(range);
	Vec2 difference = {
		.x = circle->position.x - range_center.x,
		.y = circle->position.y - range_center.y
	};
	Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((range->max.x - range->min.x) / 2),
			 ((range->max.x - range->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((range->max.y - range->min.y) / 2),
			 ((range->max.y - range->min.y) / 2))
	};
	Vec2 closest = {
		.x = range_center.x + clamped.x,
		.y = range_center.y + clamped.y
	};
	difference = (Vec2){
		.x = closest.x - circle->position.x,
		.y = closest.y - circle->position.y
	};
	return vec2_length(&difference) < circle->radius;
}

bool range_intersects_range(Range2 *r1, Range2 *r2) {
	if (r1->max.x < r2->min.x || r1->min.x >= r2->max.x ||
		r1->max.y < r2->min.y || r1->min.y >= r2->max.y) {
		return false;
	};
	return true;
}

Vec2 range_get_center(Range2 *range) {
	return (Vec2){
		.x = range->min.x + (range->max.x - range->min.x) / 2,
		.y = range->min.y + (range->max.y - range->min.y) / 2,
	};
}

