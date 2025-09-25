#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <raylib.h>

#include "quadtree.h"
#include "util.h"

#define RANDOM 1

#define TEST_RECTS 0
#define TEST_CIRCLES 1

#define TEST_TYPE TEST_RECTS

#define WINDOW_TITLE "Quadtree"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1280
#define FONT_SIZE 32

#if RANDOM
// Physics
#define ENTITY_COUNT 5000
#define ENTITY_RADIUS 3
#define VELOCITY_RANGE 200
#else
#define ENTITY_COUNT 3
#endif

#define QT_WIDTH SCREEN_WIDTH
#define QT_HEIGHT SCREEN_HEIGHT
#define TEST_FRAMES 100
#define TARGET_FPS 60
#define FIXED_UPDATE 0 // boolean

#define TARGET_DELTA (1.0 / TARGET_FPS)

int main(void) {
	QuadTree *qtree = quadtree_new(&(Rect){
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
	EntityCircle entities_circle[ENTITY_COUNT];
	EntityCircle entities_circle_future[ENTITY_COUNT];
	EntityCircle entities_circle_start[ENTITY_COUNT];
	EntityRect entities_rect[ENTITY_COUNT];
	EntityRect entities_rect_future[ENTITY_COUNT];
	EntityRect entities_rect_start[ENTITY_COUNT];
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
		entities_circle[i] = (EntityCircle){
			.velocity = start_velocities[i],
			.shape = {
				.position = {
					.x = start_positions[i].x,
					.y = start_positions[i].y,
				},
				.radius = ENTITY_RADIUS
			}
		};
		entities_rect[i] = (EntityRect){
			.velocity = start_velocities[i],
			.shape = {
				.min = {
					.x = start_positions[i].x - ENTITY_RADIUS,
					.y = start_positions[i].y - ENTITY_RADIUS,
				},
				.max = {
					.x = start_positions[i].x + ENTITY_RADIUS, 
					.y = start_positions[i].y + ENTITY_RADIUS,
				}
			}
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
	memcpy(entities_rect_future, entities_rect, sizeof(EntityRect) * ENTITY_COUNT);
	memcpy(entities_rect_start, entities_rect, sizeof(EntityRect) * ENTITY_COUNT);
	memcpy(entities_circle_future, entities_circle, sizeof(EntityCircle) * ENTITY_COUNT);
	memcpy(entities_circle_start, entities_circle, sizeof(EntityCircle) * ENTITY_COUNT);

	printf("entity count: %d\n", ENTITY_COUNT);

	timespec start_time;
	timespec end_time;
	timespec work_time;
	DynamicArray intersecting_entities;
	dynamic_array_init(&intersecting_entities);
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
			memcpy(entities_rect, entities_rect_start, sizeof(EntityRect) * ENTITY_COUNT);
			memcpy(entities_rect_future, entities_rect_start, sizeof(EntityRect) * ENTITY_COUNT);
		}
		entities_in_qtree = quadtree_add_entities_rect(qtree, entities_rect, ENTITY_COUNT);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			dynamic_array_clear(&intersecting_entities);
			quadtree_entities_rect_intersecting_entity_rect(qtree, &entities_rect[i], &intersecting_entities);
			if (intersecting_entities.size > 0) {
				Vec2 rect_center;
				Vec2 relative_velocity;
				Vec2 collision_position_sum = VEC2_ZERO;
				Vec2 relative_velocity_sum = VEC2_ZERO;
				for (j = 0; j < intersecting_entities.size; ++j) {
					EntityRect *intersecting_rect = intersecting_entities.array[j];
					rect_center = rect_get_center(&intersecting_rect->shape);
					collision_position_sum = vec2_add(&collision_position_sum, &rect_center);
					relative_velocity = vec2_subtract(&intersecting_rect->velocity, &entities_rect[i].velocity);
					relative_velocity_sum = vec2_add(&relative_velocity_sum, &relative_velocity);
				}
				relative_velocity = vec2_divide(&relative_velocity_sum, intersecting_entities.size);
				Vec2 collision_position = vec2_divide(&collision_position_sum, intersecting_entities.size);
				rect_center = rect_get_center(&entities_rect[i].shape);
				Vec2 position_difference = vec2_subtract(&collision_position, &rect_center);
				if (vec2_dot_product(&position_difference, &relative_velocity) < 0) {
					Vec2 tangent_vector = {
						.x = -position_difference.y,
						.y =  position_difference.x
					};
					tangent_vector = vec2_normalized(&tangent_vector);
					float length = vec2_dot_product(&relative_velocity, &tangent_vector);
					Vec2 velocity_on_tangent = vec2_multiply(&tangent_vector, length);
					Vec2 velocity_perpendicular_to_tangent = vec2_subtract(&relative_velocity, &velocity_on_tangent);
					entities_rect_future[i].velocity.x += velocity_perpendicular_to_tangent.x;
					entities_rect_future[i].velocity.y += velocity_perpendicular_to_tangent.y;
				}
			}
			entities_rect_future[i].shape.min.x += entities_rect_future[i].velocity.x * delta_time;
			entities_rect_future[i].shape.max.x += entities_rect_future[i].velocity.x * delta_time;
			entities_rect_future[i].shape.min.y += entities_rect_future[i].velocity.y * delta_time;
			entities_rect_future[i].shape.max.y += entities_rect_future[i].velocity.y * delta_time;
			total_collisions += intersecting_entities.size;
		}

		memcpy(entities_rect, entities_rect_future, sizeof(EntityRect) * ENTITY_COUNT);

		// Render
		BeginDrawing();
		ClearBackground(BLACK);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			DrawRectangle(
				entities_rect[i].shape.min.x, 
				entities_rect[i].shape.min.y, 
				entities_rect[i].shape.max.x - entities_rect[i].shape.min.x, 
				entities_rect[i].shape.max.y - entities_rect[i].shape.min.y,
				GREEN
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
		entities_in_qtree = quadtree_add_entities_circle(qtree, entities_circle, ENTITY_COUNT);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			dynamic_array_clear(&intersecting_entities);
			quadtree_entities_circle_intersecting_entity_circle(qtree, &entities_circle[i], &intersecting_entities);
			if (intersecting_entities.size > 0) {
				Vec2 relative_velocity;
				Vec2 collision_position_sum = VEC2_ZERO;
				Vec2 relative_velocity_sum = VEC2_ZERO;
				for (j = 0; j < intersecting_entities.size; ++j) {
					EntityCircle *intersecting_circle = intersecting_entities.array[j];
					collision_position_sum = vec2_add(&collision_position_sum, &intersecting_circle->shape.position);
					relative_velocity = vec2_subtract(&intersecting_circle->velocity, &entities_circle[i].velocity);
					relative_velocity_sum = vec2_add(&relative_velocity_sum, &relative_velocity);
				}
				relative_velocity = vec2_divide(&relative_velocity_sum, intersecting_entities.size);
				Vec2 collision_position = vec2_divide(&collision_position_sum, intersecting_entities.size);
				Vec2 position_difference = vec2_subtract(&collision_position, &entities_circle[i].shape.position);
				if (vec2_dot_product(&position_difference, &relative_velocity) < 0) {
					Vec2 tangent_vector = {
						.x = -position_difference.y,
						.y =  position_difference.x
					};
					tangent_vector = vec2_normalized(&tangent_vector);
					float length = vec2_dot_product(&relative_velocity, &tangent_vector);
					Vec2 velocity_on_tangent = vec2_multiply(&tangent_vector, length);
					Vec2 velocity_perpendicular_to_tangent = vec2_subtract(&relative_velocity, &velocity_on_tangent);
					entities_circle_future[i].velocity.x += velocity_perpendicular_to_tangent.x;
					entities_circle_future[i].velocity.y += velocity_perpendicular_to_tangent.y;
				}
			}
			entities_circle_future[i].shape.position.x += entities_circle_future[i].velocity.x * delta_time;
			entities_circle_future[i].shape.position.y += entities_circle_future[i].velocity.y * delta_time;
			total_collisions += intersecting_entities.size;
		}

		memcpy(entities_circle, entities_circle_future, sizeof(EntityCircle) * ENTITY_COUNT);

		// Render
		BeginDrawing();
		ClearBackground(BLACK);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			DrawCircle(
				entities_circle[i].shape.position.x,
				entities_circle[i].shape.position.y,
				entities_circle[i].shape.radius,
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
	free(intersecting_entities.array);
	quadtree_free(qtree);
	return 0;
}
