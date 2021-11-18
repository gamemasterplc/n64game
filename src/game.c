#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "pad.h"
#include "bool.h"
#include "libcext.h"

#define BULLET_MAX_TIME 300
#define BULLET_VEL 15.0f
#define MAX_BULLETS 50

#define SHIP_IMAGE_W 32
#define SHIP_IMAGE_H 32
#define SHIP_W 14
#define SHIP_H 21
#define SHIP_THRUST 0.1f
#define SHIP_ROT_SPEED 0.09f
#define SHIP_MAX_VEL 7.0f

typedef struct ship {
	N64Image *image;
	float angle;
	float x;
	float y;
	float vel_x;
	float vel_y;
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

static Ship ship;
static Bullet bullets[MAX_BULLETS];

static void StateInit()
{
	ship.image = ImageCreate(SHIP_IMAGE_W, SHIP_IMAGE_H, IMG_FMT_I8);
	ship.x = GfxGetWidth()/2;
	ship.y = GfxGetHeight()/2;
	ship.angle = 0;
	ship.update_image = true;
	ship.vel_x = ship.vel_y = 0;
}

static void ClearShipImage()
{
	u8 *data = ship.image->data;
	for(int i=0; i<SHIP_IMAGE_W*SHIP_IMAGE_H; i++) {
		data[i] = 0; //Write transparent black
	}
}

static void DrawShipLine(int x0, int y0, int x1, int y1)
{
	//Implementation of Bresenham line drawing algorithm
	u8 *data = ship.image->data;
	int dx =  abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1; 
	int err = dx + dy;
	int e2;
	while(1) {
		data[(y0*SHIP_IMAGE_W)+x0] = 0xFF; //Write white
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy; x0 += sx;
		}
		if (e2 <= dx) {
			err += dx; y0 += sy;
		}
	}
}

static void UpdateShipImage()
{
	float c, s;
	int x1, y1, x2, y2, x3, y3;
	ClearShipImage();
	c = cosf(ship.angle*M_DTOR);
	s = sinf(ship.angle*M_DTOR);
	//Rotate (0, -14) to ship image
	x1 = (0*c)-(-((SHIP_H*2)/3)*s);
	y1 = (0*s)+(-((SHIP_H*2)/3)*c);
	x1 += SHIP_IMAGE_W/2;
	y1 += SHIP_IMAGE_H/2;
	//Rotate (-8, 7) to ship image
	x2 = (-(SHIP_W/2)*c)-((SHIP_H/3)*s);
	y2 = (-(SHIP_W/2)*s)+((SHIP_H/3)*c);
	x2 += SHIP_IMAGE_W/2;
	y2 += SHIP_IMAGE_H/2;
	//Rotate (8, 7) to ship image
	x3 = ((SHIP_W/2)*c)-((SHIP_H/3)*s);
	y3 = ((SHIP_W/2)*s)+((SHIP_H/3)*c);
	x3 += SHIP_IMAGE_W/2;
	y3 += SHIP_IMAGE_H/2;
	//Draw ship edges
	DrawShipLine(x1, y1, x2, y2);
	DrawShipLine(x1, y1, x3, y3);
	DrawShipLine(x2, y2, x3, y3);
	ImageFlushData(ship.image);
}

static void WrapPos(int margin_x, int margin_y, float *x, float *y)
{
	float scr_w, scr_h;
	scr_w = GfxGetWidth();
	scr_h = GfxGetHeight();
	if(*x < -margin_x) {
		*x += scr_w+(margin_x*2);
	}
	if(*y < -margin_y) {
		*y += scr_h+(margin_y*2);
	}
	if(*x >= scr_w+margin_x) {
		*x -= scr_w+(margin_x*2);
	}
	if(*y >= scr_h+margin_y) {
		*y -= scr_h+(margin_y*2);
	}
}

static void CreateBullet()
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		if(!bullets[i].exists) {
			break;
		}
	}
	if(i == MAX_BULLETS) {
		return;
	}
	bullets[i].timer = BULLET_MAX_TIME;
	bullets[i].x = ship.x;
	bullets[i].y = ship.y;
	bullets[i].vel_x = BULLET_VEL*sinf(ship.angle*M_DTOR);
	bullets[i].vel_y = -BULLET_VEL*cosf(ship.angle*M_DTOR);
	bullets[i].exists = true;
}

static void UpdateShip()
{
	s8 stick_x = PadGetStickX(0);
	if(stick_x < -16 || stick_x > 16) {
		ship.angle += stick_x*SHIP_ROT_SPEED;
		ship.update_image = true;
	}
	if(PadGetHeldButtons(0) & A_BUTTON) {
		ship.vel_x += sinf(ship.angle*M_DTOR)*SHIP_THRUST;
		ship.vel_y += -cosf(ship.angle*M_DTOR)*SHIP_THRUST;
	}
	if(PadGetPressedButtons(0) & B_BUTTON) {
		CreateBullet();
	}
	if(ship.vel_x < -SHIP_MAX_VEL) {
		ship.vel_x = -SHIP_MAX_VEL;
	}
	if(ship.vel_x > SHIP_MAX_VEL) {
		ship.vel_x = SHIP_MAX_VEL;
	}
	if(ship.vel_y < -SHIP_MAX_VEL) {
		ship.vel_y = -SHIP_MAX_VEL;
	}
	if(ship.vel_y > SHIP_MAX_VEL) {
		ship.vel_y = SHIP_MAX_VEL;
	}
	ship.x += ship.vel_x;
	ship.y += ship.vel_y;
	WrapPos(SHIP_IMAGE_W/2, SHIP_IMAGE_H/2, &ship.x, &ship.y);
	if(ship.update_image) {
		UpdateShipImage();
		ship.update_image = false;
	}
}

static void UpdateBullets()
{
	for(int i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			bullets[i].timer--;
			if(bullets[i].timer == 0) {
				bullets[i].exists = false;
				continue;
			}
			bullets[i].x += bullets[i].vel_x;
			bullets[i].y += bullets[i].vel_y;
			WrapPos(0, 0, &bullets[i].x, &bullets[i].y);
		}
	}
}

static void StateMain()
{
	UpdateShip();
	UpdateBullets();
}

static void PutSpriteCenter(N64Image *image, float x, float y)
{
	ImagePut(image, x-(image->w/2), y-(image->h/2));
}

static void DrawBullets()
{
	for(int i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			GfxPutRect(bullets[i].x, bullets[i].y, 1, 1, GFX_COLOR_WHITE);
		}
	}
}

static void StateDraw()
{
	PutSpriteCenter(ship.image, ship.x, ship.y);
	DrawBullets();
}

static void StateDestroy()
{
	ImageDelete(ship.image);
}

StateEntry Game_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};