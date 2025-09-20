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

bool rect_intersects_point(Rect *rect, Vec2 *point) {
	if (point->x < rect->min.x || point->x >= rect->max.x ||
		point->y < rect->min.y || point->y >= rect->max.y) {
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

bool point_array_init(PointArray *point_array) {
	Vec2 **array = malloc(sizeof(*array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	point_array->size = 0;
	point_array->capacity = ARRAY_DEFAULT_CAPACITY;
	point_array->array = array;
	return true;
}

bool point_array_push_back(PointArray *point_array, Vec2 *point) {
	if (point_array->size == point_array->capacity) {
		Vec2 **array = realloc(point_array->array, sizeof(*array) * point_array->capacity * 2);
		if (array == NULL) {
			return false;
		}
		point_array->array = array;
		point_array->capacity *= 2;
	}
	point_array->array[point_array->size++] = point;
	return true;
}

void point_array_clear(PointArray *point_array) {
	point_array->size = 0;
}

bool rect_array_init(RectArray *rect_array) {
	Rect **array = malloc(sizeof(*array) * ARRAY_DEFAULT_CAPACITY);
	if (array == NULL) {
		return false;
	}
	rect_array->size = 0;
	rect_array->capacity = ARRAY_DEFAULT_CAPACITY;
	rect_array->array = array;
	return true;
}

bool rect_array_push_back(RectArray *rect_array, Rect *rect) {
	if (rect_array->size == rect_array->capacity) {
		Rect **array = realloc(rect_array->array, sizeof(*array) * rect_array->capacity * 2);
		if (array == NULL) {
			return false;
		}
		rect_array->array = array;
		rect_array->capacity *= 2;
	}
	rect_array->array[rect_array->size++] = rect;
	return true;
}

void rect_array_clear(RectArray *rect_array) {
	rect_array->size = 0;
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
		Circle **array = realloc(circle_array->array, sizeof(*array) * circle_array->capacity * 2);
		if (array == NULL) {
			return false;
		}
		circle_array->array = array;
		circle_array->capacity *= 2;
	}
	circle_array->array[circle_array->size++] = circle;
	return true;
}

void circle_array_clear(CircleArray *circle_array) {
	circle_array->size = 0;
}

bool rect_intersects_circle(Rect *rect, Circle *circle) {
	Vec2 rect_center = rect_get_center(rect);
	Vec2 difference = {
		.x = circle->position.x - rect_center.x,
		.y = circle->position.y - rect_center.y
	};
	Vec2 clamped = {
		.x = clamp_float(difference.x,
			-((rect->max.x - rect->min.x) / 2),
			 ((rect->max.x - rect->min.x) / 2)),
		.y = clamp_float(difference.y,
			-((rect->max.y - rect->min.y) / 2),
			 ((rect->max.y - rect->min.y) / 2))
	};
	Vec2 closest = {
		.x = rect_center.x + clamped.x,
		.y = rect_center.y + clamped.y
	};
	difference = (Vec2){
		.x = closest.x - circle->position.x,
		.y = closest.y - circle->position.y
	};
	return vec2_length(&difference) < circle->radius;
}

bool rect_intersects_rect(Rect *r1, Rect *r2) {
	if (r1->max.x < r2->min.x || r1->min.x >= r2->max.x ||
		r1->max.y < r2->min.y || r1->min.y >= r2->max.y) {
		return false;
	};
	return true;
}

Vec2 rect_get_center(Rect *rect) {
	return (Vec2){
		.x = rect->min.x + (rect->max.x - rect->min.x) / 2,
		.y = rect->min.y + (rect->max.y - rect->min.y) / 2,
	};
}

