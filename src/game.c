#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "pad.h"
#include "bool.h"
#include "libcext.h"


#define SHIP_IMAGE_W 32
#define SHIP_IMAGE_H 32
#define SHIP_HALF_W 8
#define SHIP_HALF_H 12
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
} Ship;

static Ship ship;

static void StateInit()
{
	ship.image = ImageCreate(SHIP_IMAGE_W, SHIP_IMAGE_H, IMG_FMT_I8);
	ship.x = GfxGetWidth()/2;
	ship.y = GfxGetHeight()/2;
	ship.angle = 0;
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
	float x1, y1, x2, y2, x3, y3;
	ClearShipImage();
	c = cosf(ship.angle*M_DTOR);
	s = sinf(ship.angle*M_DTOR);
	//Rotate (0, -12) to ship image
	x1 = (0*c)-(-SHIP_HALF_H*s);
	y1 = (0*s)+(-SHIP_HALF_H*c);
	x1 += SHIP_IMAGE_W/2;
	y1 += SHIP_IMAGE_H/2;
	//Rotate (-8, 12) to ship image
	x2 = (-SHIP_HALF_W*c)-(SHIP_HALF_H*s);
	y2 = (-SHIP_HALF_W*s)+(SHIP_HALF_H*c);
	x2 += SHIP_IMAGE_W/2;
	y2 += SHIP_IMAGE_H/2;
	//Rotate (8, 12) to ship image
	x3 = (SHIP_HALF_W*c)-(SHIP_HALF_H*s);
	y3 = (SHIP_HALF_W*s)+(SHIP_HALF_H*c);
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

static void UpdateShip()
{
	s8 stick_x = PadGetStickX(0);
	if(stick_x < -16 || stick_x > 16) {
		ship.angle += stick_x*SHIP_ROT_SPEED;
	}
	if(PadGetHeldButtons(0) & A_BUTTON) {
		ship.vel_x += sinf(ship.angle*M_DTOR)*SHIP_THRUST;
		ship.vel_y += -cosf(ship.angle*M_DTOR)*SHIP_THRUST;
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
	UpdateShipImage();
}

static void StateMain()
{
	UpdateShip();
}

static void PutSpriteCenter(N64Image *image, float x, float y)
{
	ImagePut(image, x-(image->w/2), y-(image->h/2));
}

static void StateDraw()
{
	PutSpriteCenter(ship.image, ship.x, ship.y);
}

static void StateDestroy()
{
	ImageDelete(ship.image);
}

StateEntry Game_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};