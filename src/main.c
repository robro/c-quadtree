#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <raylib.h>
#include <pthread.h>

#include "quadtree.h"
#include "util.h"

#define RANDOM 1

#define TEST_RECTS 0
#define TEST_CIRCLES 1

#define TEST_TYPE TEST_CIRCLES

#define WINDOW_TITLE "Quadtree"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1280
#define FONT_SIZE 32

#if RANDOM
// Physics
#define ENTITY_COUNT 4000
#define ENTITY_RADIUS 4
#define VELOCITY_RANGE 200
#else
#define ENTITY_COUNT 3
#endif

#define QT_WIDTH SCREEN_WIDTH
#define QT_HEIGHT SCREEN_HEIGHT
#define TEST_FRAMES 100
#define TARGET_FPS 60
#define FIXED_UPDATE 0 // boolean
#define THREAD_COUNT 8

#define TARGET_DELTA (1.0 / TARGET_FPS)

typedef void (*QTreeIntersectsFunc)(const QuadTree *, const Entity *, DynamicArray *);

typedef struct PhysicsUpdateArgs {
	const QuadTree *qtree;
	const Entity *entities;
	Entity *entities_future;
	uint entity_count;
	float delta_time;
	QTreeIntersectsFunc intersect_func;
} PhysicsUpdateArgs;

void *update_physics(void *args) {
	PhysicsUpdateArgs *_args = args;
	DynamicArray intersecting;
	dynamic_array_init(&intersecting);

	for (int i = 0; i < _args->entity_count; ++i) {
		_args->intersect_func(_args->qtree, &_args->entities[i], &intersecting);
		if (intersecting.size > 0) {
			Vec2 relative_velocity;
			Vec2 collision_position_sum = VEC2_ZERO;
			Vec2 relative_velocity_sum = VEC2_ZERO;
			for (int j = 0; j < intersecting.size; ++j) {
				Entity *intersecting_entity = intersecting.array[j];
				collision_position_sum = vec2_add(&collision_position_sum, &intersecting_entity->position);
				relative_velocity = vec2_subtract(&intersecting_entity->velocity, &_args->entities[i].velocity);
				relative_velocity_sum = vec2_add(&relative_velocity_sum, &relative_velocity);
			}
			relative_velocity = vec2_divide(&relative_velocity_sum, intersecting.size);
			Vec2 collision_position = vec2_divide(&collision_position_sum, intersecting.size);
			Vec2 position_difference = vec2_subtract(&collision_position, &_args->entities[i].position);
			if (vec2_dot_product(&position_difference, &relative_velocity) < 0) {
				Vec2 tangent_vector = {
					.x = -position_difference.y,
					.y =  position_difference.x
				};
				tangent_vector = vec2_normalized(&tangent_vector);
				float length = vec2_dot_product(&relative_velocity, &tangent_vector);
				Vec2 velocity_on_tangent = vec2_multiply(&tangent_vector, length);
				Vec2 velocity_perpendicular_to_tangent = vec2_subtract(&relative_velocity, &velocity_on_tangent);
				_args->entities_future[i].velocity.x += velocity_perpendicular_to_tangent.x;
				_args->entities_future[i].velocity.y += velocity_perpendicular_to_tangent.y;
			}
		}
		_args->entities_future[i].position.x += _args->entities_future[i].velocity.x * _args->delta_time;
		_args->entities_future[i].position.y += _args->entities_future[i].velocity.y * _args->delta_time;
		dynamic_array_clear(&intersecting);
	}
	dynamic_array_free(&intersecting);
	return NULL;
}

int main(void) {
	QuadTree *qtree = quadtree_new(&(AABB){
		.min = {.x = 0, .y = 0},
		.max = {.x = QT_WIDTH, .y = QT_HEIGHT},
	});
	if (qtree == NULL) {
		printf("ERROR: Failed to create quadtree!\n");
		return 1;
	}

	srand(time(0));
	Vec2 start_positions[ENTITY_COUNT];
	Vec2 start_velocities[ENTITY_COUNT];
	Entity entities_circle[ENTITY_COUNT];
	Entity entities_circle_future[ENTITY_COUNT];
	Entity entities_circle_start[ENTITY_COUNT];
	Entity entities_rect[ENTITY_COUNT];
	Entity entities_rect_future[ENTITY_COUNT];
	Entity entities_rect_start[ENTITY_COUNT];
	pthread_t pthreads[THREAD_COUNT];
	PhysicsUpdateArgs *physics_args[THREAD_COUNT];
	int i, j, k;

#if RANDOM
	for (i = 0; i < ENTITY_COUNT; ++i) {
		start_positions[i] = (Vec2){
			.x = (float)rand() / RAND_MAX * QT_WIDTH / 2 + (float)QT_WIDTH / 4,
			.y = (float)rand() / RAND_MAX * QT_HEIGHT / 2 + (float)QT_HEIGHT / 4,
		};
		start_velocities[i] = (Vec2){
			.x = ((float)rand() / RAND_MAX - 0.5) * VELOCITY_RANGE,
			.y = ((float)rand() / RAND_MAX - 0.5) * VELOCITY_RANGE,
		},
		entities_circle[i] = (Entity){
			.position = start_positions[i],
			.velocity = start_velocities[i],
			.shape.circle.radius = ENTITY_RADIUS,
		};
		entities_rect[i] = (Entity){
			.position = start_positions[i],
			.velocity = start_velocities[i],
			.shape.rect.width = ENTITY_RADIUS * 2,
			.shape.rect.height = ENTITY_RADIUS * 2,
		};
	}
#else
	entities_circle[0] = (EntityCircle){
		.velocity = {
			.x = 200,
			.y = 0,
		},
		.shape = (Circle){
			.position = {
				.x = 200,
				.y = (float)QT_HEIGHT / 2,
			},
			.radius = 100,
		}
	};
	entities_circle[1] = (EntityCircle){
		.velocity = {
			.x = 0,
			.y = 0,
		},
		.shape = (Circle){
			.position = {
				.x = 600,
				.y = (float)QT_HEIGHT / 2 - 100,
			},
			.radius = 100,
		}
	};
	entities_circle[2] = (EntityCircle){
		.velocity = {
			.x = 0,
			.y = 0,
		},
		.shape = (Circle){
			.position = {
				.x = 600,
				.y = (float)QT_HEIGHT / 2 + 100,
			},
			.radius = 100,
		}
	};
#endif
	memcpy(entities_rect_future, entities_rect, sizeof(Entity) * ENTITY_COUNT);
	memcpy(entities_rect_start, entities_rect, sizeof(Entity) * ENTITY_COUNT);
	memcpy(entities_circle_future, entities_circle, sizeof(Entity) * ENTITY_COUNT);
	memcpy(entities_circle_start, entities_circle, sizeof(Entity) * ENTITY_COUNT);

	printf("entity count: %d\n", ENTITY_COUNT);

	timespec start_time;
	timespec end_time;
	timespec work_time;
	uint total_collisions;

	char entity_count_str[32];
	char fps_str[32];
	char frame_time_str[32];

	float delta_time = TARGET_DELTA;

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
#if FIXED_UPDATE
	SetTargetFPS(TARGET_FPS);
#endif

	while (!WindowShouldClose()) {
		uint entities_in_qtree;
		quadtree_clear(qtree);
#if TEST_TYPE == TEST_RECTS
		if (IsKeyPressed(KEY_SPACE)) {
			memcpy(entities_rect, entities_rect_start, sizeof(Entity) * ENTITY_COUNT);
			memcpy(entities_rect_future, entities_rect_start, sizeof(Entity) * ENTITY_COUNT);
		}
		entities_in_qtree = quadtree_add_entities_rect(qtree, entities_rect, ENTITY_COUNT);
		for (i = 0; i < THREAD_COUNT; ++i) {
			physics_args[i] = malloc(sizeof(PhysicsUpdateArgs));
			if (physics_args[i] == NULL) {
				return 1;
			}
			*physics_args[i] = (PhysicsUpdateArgs){
				.qtree = qtree,
				.entities = &entities_rect[ENTITY_COUNT / THREAD_COUNT * i],
				.entities_future = &entities_rect_future[ENTITY_COUNT / THREAD_COUNT * i],
				.entity_count = ENTITY_COUNT / THREAD_COUNT,
				.delta_time = delta_time,
				.intersect_func = quadtree_entities_rect_intersecting_entity_rect,
			};
			pthread_create(&pthreads[i], NULL, update_physics, physics_args[i]);
		}
		for (i = 0; i < THREAD_COUNT; ++i) {
			pthread_join(pthreads[i], NULL);
			free(physics_args[i]);
		}
		memcpy(entities_rect, entities_rect_future, sizeof(Entity) * ENTITY_COUNT);

		// Render
		BeginDrawing();
		ClearBackground(BLACK);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			DrawCircle(
				entities_rect[i].position.x,
				entities_rect[i].position.y,
				ENTITY_RADIUS,
				RED
			);
		}
		sprintf(entity_count_str, "entities: %d", entities_in_qtree);
		sprintf(fps_str, "fps: %d", GetFPS());
		sprintf(frame_time_str, "frame time: %f", GetFrameTime());
		DrawText(entity_count_str, 0, 0, FONT_SIZE, WHITE);
		DrawText(fps_str, 0, FONT_SIZE, FONT_SIZE, WHITE);
		DrawText(frame_time_str, 0, FONT_SIZE * 2, FONT_SIZE, WHITE);
		EndDrawing();

#elif TEST_TYPE == TEST_CIRCLES
		if (IsKeyPressed(KEY_SPACE)) {
			memcpy(entities_circle, entities_circle_start, sizeof(Entity) * ENTITY_COUNT);
			memcpy(entities_circle_future, entities_circle_start, sizeof(Entity) * ENTITY_COUNT);
		}
		entities_in_qtree = quadtree_add_entities_circle(qtree, entities_circle, ENTITY_COUNT);
		for (i = 0; i < THREAD_COUNT; ++i) {
			physics_args[i] = malloc(sizeof(PhysicsUpdateArgs));
			if (physics_args[i] == NULL) {
				return 1;
			}
			*physics_args[i] = (PhysicsUpdateArgs){
				.qtree = qtree,
				.entities = &entities_circle[ENTITY_COUNT / THREAD_COUNT * i],
				.entities_future = &entities_circle_future[ENTITY_COUNT / THREAD_COUNT * i],
				.entity_count = ENTITY_COUNT / THREAD_COUNT,
				.delta_time = delta_time,
				.intersect_func = quadtree_entities_circle_intersecting_entity_circle,
			};
			pthread_create(&pthreads[i], NULL, update_physics, physics_args[i]);
		}
		for (i = 0; i < THREAD_COUNT; ++i) {
			pthread_join(pthreads[i], NULL);
			free(physics_args[i]);
		}
		memcpy(entities_circle, entities_circle_future, sizeof(Entity) * ENTITY_COUNT);

		// Render
		BeginDrawing();
		ClearBackground(BLACK);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			DrawCircle(
				entities_circle[i].position.x,
				entities_circle[i].position.y,
				entities_circle[i].shape.circle.radius,
				BLUE
			);
		}
		sprintf(entity_count_str, "entities: %d", entities_in_qtree);
		sprintf(fps_str, "fps: %d", GetFPS());
		sprintf(frame_time_str, "frame time: %f", GetFrameTime());
		DrawText(entity_count_str, 0, 0, FONT_SIZE, WHITE);
		DrawText(fps_str, 0, FONT_SIZE, FONT_SIZE, WHITE);
		DrawText(frame_time_str, 0, FONT_SIZE * 2, FONT_SIZE, WHITE);
		EndDrawing();
#endif
	}

	CloseWindow();
	quadtree_free(qtree);
	return 0;
}
