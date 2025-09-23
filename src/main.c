#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <raylib.h>

#include "quadtree.h"
#include "util.h"

#define TEST_RECTS 0
#define TEST_CIRCLES 1

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define WINDOW_TITLE "Quadtree"
#define FONT_SIZE 32

// Physics
#define ENTITY_COUNT 2500
#define ENTITY_RADIUS 5
#define VELOCITY_RANGE 200

#define QT_WIDTH 800
#define QT_HEIGHT 800
#define TEST_FRAMES 100
#define TARGET_FPS 60

#define FIXED_DELTA_TIME (1.0 / TARGET_FPS)

int main(void) {
	QuadTree *qtree = quadtree_new(&(Rect){
		.min = {.x = 0, .y = 0},
		.max = {.x = QT_WIDTH, .y = QT_HEIGHT},
	});
	if (qtree == NULL) {
		printf("ERROR: Failed to create quadtree!\n");
		return 1;
	}

	srand(0);
	Vec2 points[ENTITY_COUNT];
	Rect rects[ENTITY_COUNT];
	Circle circles[ENTITY_COUNT];
	EntityCircle entities_circle[ENTITY_COUNT];
	EntityCircle entities_circle_future[ENTITY_COUNT];
	int i, j, k;

	for (i = 0; i < ENTITY_COUNT; ++i) {
		points[i] = (Vec2){
			.x = (float)rand() / RAND_MAX * QT_WIDTH,
			.y = (float)rand() / RAND_MAX * QT_HEIGHT
		};
		rects[i] = (Rect){
			.min = {.x = points[i].x - ENTITY_RADIUS, .y = points[i].y - ENTITY_RADIUS},
			.max = {.x = points[i].x + ENTITY_RADIUS, .y = points[i].y + ENTITY_RADIUS}
		};
		circles[i] = (Circle){
			.position = {.x = points[i].x, .y = points[i].y},
			.radius = ENTITY_RADIUS
		};
		entities_circle[i] = (EntityCircle){
			.velocity = {
				.x = ((float)rand() / RAND_MAX - 0.5) * VELOCITY_RANGE,
				.y = ((float)rand() / RAND_MAX - 0.5) * VELOCITY_RANGE
			},
			.shape = circles[i]
		};
	}
	printf("entity count: %d\n", ENTITY_COUNT);
	memcpy(entities_circle_future, entities_circle, sizeof(EntityCircle) * ENTITY_COUNT);

	timespec start_time;
	timespec end_time;
	timespec work_time;
	DynamicArray collisions;
	dynamic_array_init(&collisions);
	uint total_collisions;

	EntityCircle *colliding_circle;
	Vec2 collision_normal;
	Vec2 collision_normal_sum;
	float dot_product;
	Vec2 new_velocity;

	float delta_time;
	char fps_str[32];
	char frame_time_str[32];

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
	// SetTargetFPS(TARGET_FPS);

	while (!WindowShouldClose()) {
#if TEST_RECTS
		quadtree_add_rects(qtree, rects, ENTITY_COUNT);
		for (j = 0; j < ENTITY_COUNT; ++j) {
			dynamic_array_clear(&collisions);
			quadtree_rects_intersecting_rect(qtree, &rects[j], &collisions);
			total_collisions += collisions.size;
		}
		quadtree_clear(qtree);
#endif

#if TEST_CIRCLES
		delta_time = GetFrameTime();
		quadtree_clear(qtree);
		quadtree_add_entities_circle(qtree, entities_circle, ENTITY_COUNT);
		for (j = 0; j < ENTITY_COUNT; ++j) {
			dynamic_array_clear(&collisions);
			quadtree_entities_circle_intersecting_entity_circle(qtree, &entities_circle[j], &collisions);
			if (collisions.size > 0) {
				collision_normal_sum = (Vec2){0, 0};
				for (k = 0; k < collisions.size; ++k) {
					colliding_circle = collisions.array[k];
					collision_normal = vec2_direction(&colliding_circle->shape.position, &entities_circle[j].shape.position);
					collision_normal_sum = vec2_add(&collision_normal_sum, &collision_normal);
				}
				collision_normal = vec2_normalized(&collision_normal_sum);
				dot_product = vec2_dot_product(&entities_circle[j].velocity, &collision_normal);
				if (dot_product < 0) {
					new_velocity = vec2_multiply(&collision_normal, 2 * dot_product);
					new_velocity = vec2_subtract(&entities_circle[j].velocity, &new_velocity);
					entities_circle_future[j].velocity = new_velocity;
				}
			}
			entities_circle_future[j].shape.position.x += entities_circle_future[j].velocity.x * FIXED_DELTA_TIME;
			entities_circle_future[j].shape.position.y += entities_circle_future[j].velocity.y * FIXED_DELTA_TIME;
			total_collisions += collisions.size;
		}

		memcpy(entities_circle, entities_circle_future, sizeof(EntityCircle) * ENTITY_COUNT);

		// Render
		BeginDrawing();
		ClearBackground(BLACK);
		for (i = 0; i < ENTITY_COUNT; ++i) {
			DrawCircle(entities_circle[i].shape.position.x, entities_circle[i].shape.position.y, entities_circle[i].shape.radius, BLUE);
		}
		sprintf(fps_str, "fps: %d", GetFPS());
		sprintf(frame_time_str, "frame time: %f", delta_time);
		DrawText(fps_str, 0, 0, FONT_SIZE, WHITE);
		DrawText(frame_time_str, 0, FONT_SIZE, FONT_SIZE, WHITE);
		EndDrawing();
#endif
	}

	CloseWindow();
	free(collisions.array);
	quadtree_free(qtree);
	return 0;
}
