#include <math.h>
#include <stdlib.h>

#include "util.h"

#define ARRAY_DEFAULT_CAPACITY 2

struct timespec timespec_diff(const timespec *time1, const timespec *time2) {
	timespec diff = {
		.tv_sec = time1->tv_sec - time2->tv_sec,
		.tv_nsec = time1->tv_nsec - time2->tv_nsec,
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

float vec2_length(Vec2 *vec) {
	return sqrt(vec->x * vec->x + vec->y * vec->y);
}

Vec2 rect_get_center(Rect *rect) {
	return (Vec2){
		.x = rect->min.x + (rect->max.x - rect->min.x) / 2,
		.y = rect->min.y + (rect->max.y - rect->min.y) / 2,
	};
}

typedef struct {
	uint size;
	uint capacity;
	void **array;
} VoidArray;

static inline bool void_array_init(void *array) {
	void **new_array = malloc(sizeof(array) * ARRAY_DEFAULT_CAPACITY);
	if (new_array == NULL) {
		return false;
	}
	VoidArray *void_array = array;
	void_array->size = 0;
	void_array->capacity = ARRAY_DEFAULT_CAPACITY;
	void_array->array = new_array;
	return true;
}

static inline bool void_array_push_back(void *array, void *value) {
	VoidArray *void_array = array;
	if (void_array->size == void_array->capacity) {
		void **new_array = realloc(void_array->array, sizeof(void *) * void_array->capacity * 2);
		if (new_array == NULL) {
			return false;
		}
		void_array->array = new_array;
		void_array->capacity *= 2;
	}
	void_array->array[void_array->size++] = value;
	return true;
}

static inline void void_array_clear(void *array) {
	VoidArray *void_array = array;
	void_array->size = 0;
}

bool point_array_init(PointArray *point_array) {
	return void_array_init(point_array);
}

bool point_array_push_back(PointArray *point_array, Vec2 *point) {
	return void_array_push_back(point_array, point);
}

void point_array_clear(PointArray *point_array) {
	void_array_clear(point_array);
}

bool rect_array_init(RectArray *rect_array) {
	return void_array_init(rect_array);
}

bool rect_array_push_back(RectArray *rect_array, Rect *rect) {
	return void_array_push_back(rect_array, rect);
}

void rect_array_clear(RectArray *rect_array) {
	void_array_clear(rect_array);
}

bool circle_array_init(CircleArray *circle_array) {
	return void_array_init(circle_array);
}

bool circle_array_push_back(CircleArray *circle_array, Circle *circle) {
	return void_array_push_back(circle_array, circle);
}

void circle_array_clear(CircleArray *circle_array) {
	void_array_clear(circle_array);
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

bool rect_intersects_point(Rect *rect, Vec2 *point) {
	if (point->x < rect->min.x || point->x >= rect->max.x ||
		point->y < rect->min.y || point->y >= rect->max.y) {
		return false;
	}
	return true;
}

bool circle_intersects_circle(Circle *circle1, Circle *circle2) {
	Vec2 difference = {
		.x = circle1->position.x - circle2->position.x,
		.y = circle1->position.y - circle2->position.y,
	};
	return vec2_length(&difference) < circle1->radius + circle2->radius;
}

