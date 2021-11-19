#pragma once

#include "image.h"
#include "bool.h"

#define ASTEROID_RESPAWN_RADIUS 64
#define ASTEROID_VEL 1.0f
#define ASTEROID_NUM_SIZES 3
#define ASTEROID_SIZE 64
#define ASTEROID_ROT_STEPS 64
#define ASTEROID_ROT_DELAY 3
#define ASTEROID_SIDE_COUNT 16
#define ASTEROID_RAD_NOISE 0.4
#define MAX_ASTEROIDS (2 << (ASTEROID_NUM_SIZES-1))

#define BULLET_MAX_TIME 150
#define BULLET_VEL 7.5f
#define MAX_BULLETS 15

#define SHIP_IMAGE_W 32
#define SHIP_IMAGE_H 32
#define SHIP_W 14
#define SHIP_H 21
#define SHIP_THRUST 0.1f
#define SHIP_ROT_SPEED 0.045f
#define SHIP_MAX_VEL 3.5f
#define SHIP_INVULNERABLE_LEN 60
#define SHIP_NUM_LIVES 5

#define TEXT_H 12
#define X_MARGIN 22
#define Y_MARGIN 16

#define SCORE_X X_MARGIN
#define SCORE_Y Y_MARGIN
#define LIFE_X X_MARGIN
#define LIFE_Y SCORE_Y+TEXT_H

typedef struct ship {
	N64Image *image;
	float angle;
	float x;
	float y;
	float vel_x;
	float vel_y;
	unsigned int lives;
	int invulnerable_timer;
	bool update_image;
} Ship;

typedef struct bullet {
	bool exists;
	int timer; //Disappears when zero
	float x;
	float y;
	float vel_x;
	float vel_y;
} Bullet;

typedef struct asteroid {
	bool exists;
	int size;
	int exist_timer;
	float x;
	float y;
	float vel_x;
	float vel_y;
} Asteroid;