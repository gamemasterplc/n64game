#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "pad.h"
#include "bool.h"
#include "libcext.h"

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

typedef struct ship {
	N64Image *image;
	float angle;
	float x;
	float y;
	float vel_x;
	float vel_y;
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

static Ship ship;
static N64Image *asteroid_images[ASTEROID_NUM_SIZES][ASTEROID_ROT_STEPS];
static Asteroid asteroids[MAX_ASTEROIDS];
static Bullet bullets[MAX_BULLETS];

static void DrawImageLine(N64Image *image, int x0, int y0, int x1, int y1)
{
	//Implementation of Bresenham line drawing algorithm
	u8 *data = image->data;
	int dx =  abs(x1-x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1-y0);
	int sy = y0 < y1 ? 1 : -1; 
	int err = dx + dy;
	int e2;
	while(1) {
		data[(y0*image->w)+x0] = 0xFF; //Write white
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

static void ClearImage(N64Image *image)
{
	u8 *data = image->data;
	for(int i=0; i<image->w*image->h; i++) {
		data[i] = 0; //Write transparent black
	}
}

static void GenerateAsteroidPolygon(float *side_x, float *side_y, float center_x, float center_y, float max_radius)
{
	for(int i=0; i<ASTEROID_SIDE_COUNT; i++) {
		//Randomize radius for jagged edge
		float radius = ((((double)rand()/RAND_MAX)*ASTEROID_RAD_NOISE)+(1-ASTEROID_RAD_NOISE))*max_radius;
		float angle = (i*(360.0f/ASTEROID_SIDE_COUNT));
		//Calculate sides for approximate circle
		float x = sinf(angle*M_DTOR)*radius;
		float y = -cosf(angle*M_DTOR)*radius;
		side_x[i] = x+center_x;
		side_y[i] = y+center_y;
	}
	//Connect polygon to start point
	side_x[ASTEROID_SIDE_COUNT] = side_x[0];
	side_y[ASTEROID_SIDE_COUNT] = side_y[0];
}

static void RotatePoints(float angle, float center_x, float center_y, float *x, float *y, int num_points)
{
	float c = cosf(angle*M_DTOR);
	float s = sinf(angle*M_DTOR);
	for(int i=0; i<num_points; i++) {
		//Do standard point rotation around a center
		//Requires two temporaries if you don't want a separate buffer
		float temp_x = ((x[i]-center_x)*c)-((y[i]-center_y)*s)+center_x;
		float temp_y = ((x[i]-center_x)*s)+((y[i]-center_y)*c)+center_y;
		x[i] = temp_x;
		y[i] = temp_y;
	}
}

static void GenerateAsteroidImages()
{
	float side_x[ASTEROID_SIDE_COUNT+1];
	float side_y[ASTEROID_SIDE_COUNT+1];
	int size = ASTEROID_SIZE;
	GenerateAsteroidPolygon(side_x, side_y, size/2, size/2, size/2);
	for(int i=0; i<ASTEROID_NUM_SIZES; i++) {
		for(int j=0; j<ASTEROID_ROT_STEPS; j++) {
			//Initialize asteroid
			asteroid_images[i][j] = ImageCreate(size, size, IMG_FMT_I8);
			ClearImage(asteroid_images[i][j]);
			//Draw asteroid edges
			for(int k=0; k<ASTEROID_SIDE_COUNT; k++) {
				DrawImageLine(asteroid_images[i][j], side_x[k], side_y[k], side_x[k+1], side_y[k+1]);
			}
			//Refresh asteroid
			ImageFlushData(asteroid_images[i][j]);
			//Goto next side of asteroid
			RotatePoints(360.0f/ASTEROID_ROT_STEPS, size/2, size/2, side_x, side_y, ASTEROID_SIDE_COUNT+1);
		}
		//Unrotate asteroid
		RotatePoints(-360.0f, size/2, size/2, side_x, side_y, ASTEROID_SIDE_COUNT+1);
		for(int j=0; j<ASTEROID_SIDE_COUNT+1; j++) {
			//Halve side coordinates
			side_x[j] /= 2;
			side_y[j] /= 2;
		}
		//Halve image size
		size /= 2;
	}
}

static void ResetShip()
{
	//Center ship
	ship.x = GfxGetWidth()/2;
	ship.y = GfxGetHeight()/2;
	//Start ship facing up
	ship.angle = 0;
	ship.update_image = true;
	//Stop ship
	ship.vel_x = ship.vel_y = 0;
	ship.invulnerable_timer = SHIP_INVULNERABLE_LEN;
}

static void InitShip()
{
	ship.image = ImageCreate(SHIP_IMAGE_W, SHIP_IMAGE_H, IMG_FMT_I8);
	ResetShip();
}

static void ResetBullets()
{
	for(int i=0; i<MAX_BULLETS; i++) {
		bullets[i].exists = false;
	}
}

static void MakeAsteroid(int size, float x, float y, float vel_x, float vel_y)
{
	int i;
	for(i=0; i<MAX_ASTEROIDS; i++) {
		if(!asteroids[i].exists) {
			break;
		}
	}
	if(i == MAX_ASTEROIDS) {
		return;
	}
	asteroids[i].exists = true;
	asteroids[i].size = size;
	asteroids[i].x = x;
	asteroids[i].y = y;
	asteroids[i].vel_x = vel_x;
	asteroids[i].vel_y = vel_y;
	asteroids[i].exist_timer = 0;
}

static void InitAsteroids()
{
	for(int i=0; i<MAX_ASTEROIDS; i++) {
		asteroids[i].exists = false;
	}
	MakeAsteroid(0, (ASTEROID_SIZE*6)/8, (ASTEROID_SIZE*6)/8, (ASTEROID_VEL*4)/5, -(ASTEROID_VEL*3)/5);
	MakeAsteroid(0, GfxGetWidth()-(ASTEROID_SIZE*6)/8, (ASTEROID_SIZE*6)/8, (ASTEROID_VEL*7)/8, -(ASTEROID_VEL*1)/2);
}

static void StateInit()
{
	GenerateAsteroidImages();
	InitShip();
	ResetBullets();
	InitAsteroids();
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

static void MakeBullet()
{
	int i;
	for(i=0; i<MAX_BULLETS; i++) {
		if(!bullets[i].exists) {
			//Bullet found
			break;
		}
	}
	if(i == MAX_BULLETS) {
		//Out of bullets
		return;
	}
	bullets[i].timer = BULLET_MAX_TIME;
	bullets[i].x = ship.x;
	bullets[i].y = ship.y;
	bullets[i].vel_x = BULLET_VEL*sinf(ship.angle*M_DTOR);
	bullets[i].vel_y = -BULLET_VEL*cosf(ship.angle*M_DTOR);
	bullets[i].exists = true;
}

static void UpdateShipImage()
{
	float c, s;
	//Ship point arrays
	float ship_points_x[3] = { (SHIP_IMAGE_W/2), (-SHIP_W/2)+(SHIP_IMAGE_W/2), (SHIP_W/2)+(SHIP_IMAGE_W/2) };
	float ship_points_y[3] = { (SHIP_IMAGE_H/2)+(-(SHIP_H*2)/3), (SHIP_IMAGE_H/2)+(SHIP_H/3), (SHIP_IMAGE_H/2)+(SHIP_H/3) };
	ClearImage(ship.image);
	//Calculate ship points
	RotatePoints(ship.angle, ship.image->w/2, ship.image->h/2, ship_points_x, ship_points_y, 3);
	//Draw ship edges
	DrawImageLine(ship.image, ship_points_x[0], ship_points_y[0], ship_points_x[1], ship_points_y[1]);
	DrawImageLine(ship.image, ship_points_x[0], ship_points_y[0], ship_points_x[2], ship_points_y[2]);
	DrawImageLine(ship.image, ship_points_x[1], ship_points_y[1], ship_points_x[2], ship_points_y[2]);
	ImageFlushData(ship.image);
}

static bool IsPointInCircle(float x, float y, float circle_x, float circle_y, float radius)
{
	float dx = x-circle_x;
	float dy = y-circle_y;
	return (dx*dx)+(dy*dy) < radius*radius;
}

static void UpdateShip()
{
	s8 stick_x = PadGetStickX(0);
	if(stick_x < -16 || stick_x > 16) {
		//Cause ship rotation
		ship.angle += stick_x*SHIP_ROT_SPEED;
		ship.update_image = true;
	}
	if(PadGetHeldButtons(0) & A_BUTTON) {
		//Apply ship thrust
		ship.vel_x += sinf(ship.angle*M_DTOR)*SHIP_THRUST;
		ship.vel_y += -cosf(ship.angle*M_DTOR)*SHIP_THRUST;
	}
	if(PadGetPressedButtons(0) & B_BUTTON) {
		MakeBullet();
	}
	//Clamp ship velocity
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
	//Move Ship
	ship.x += ship.vel_x;
	ship.y += ship.vel_y;
	//Wrap ship
	WrapPos(SHIP_IMAGE_W/2, SHIP_IMAGE_H/2, &ship.x, &ship.y);
	if(ship.invulnerable_timer == 0) {
		for(int i=0; i<MAX_ASTEROIDS; i++) {
			if(asteroids[i].exists) {
				float size = ASTEROID_SIZE >> asteroids[i].size;
				size *= (1-(ASTEROID_RAD_NOISE/2));
				if(IsPointInCircle(ship.x, ship.y, asteroids[i].x, asteroids[i].y, size/2)) {
					ResetShip();
				}
			}
		}
	} else {
		ship.invulnerable_timer--;
	}
	if(ship.update_image) {
		//Refresh ship if needed
		UpdateShipImage();
		ship.update_image = false;
	}
}

static void UpdateBullets()
{
	for(int i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			bullets[i].timer--; //Tick timer
			if(bullets[i].timer == 0) {
				//Bullet is gone
				bullets[i].exists = false;
				continue;
			}
			for(int j=0; j<MAX_ASTEROIDS; j++) {
				if(asteroids[j].exists) {
					//Try to blow up asteroid
					float size = ASTEROID_SIZE >> asteroids[j].size;
					size *= (1-(ASTEROID_RAD_NOISE/2));
					if(IsPointInCircle(bullets[i].x, bullets[i].y, asteroids[j].x, asteroids[j].y, size/2)) {
						//Asteroid hit
						if(asteroids[j].size+1 < ASTEROID_NUM_SIZES) {
							//Create two child asteroids with random velocity vectors
							//at asteroid's location
							float angle1 = ((double)rand()/RAND_MAX)*360*M_DTOR;
							float angle2 = ((double)rand()/RAND_MAX)*360*M_DTOR;
							float vel_x1 = ASTEROID_VEL*cosf(angle1);
							float vel_y1 = -ASTEROID_VEL*cosf(angle1);
							float vel_x2 = ASTEROID_VEL*cosf(angle2);
							float vel_y2 = -ASTEROID_VEL*cosf(angle2);
							MakeAsteroid(asteroids[j].size+1, asteroids[j].x, asteroids[j].y, vel_x1, vel_y1);
							MakeAsteroid(asteroids[j].size+1, asteroids[j].x, asteroids[j].y, vel_x2, vel_y2);
						}
						//Destroy asteroid and bullet
						asteroids[j].exists = false;
						bullets[i].exists = false;
						goto end;
					}
				}
			}
			//Move bullet
			bullets[i].x += bullets[i].vel_x;
			bullets[i].y += bullets[i].vel_y;
			//Wrap bullet
			WrapPos(0, 0, &bullets[i].x, &bullets[i].y);
		}
		end:
	}
}

static void UpdateAsteroids()
{
	for(int i=0; i<MAX_ASTEROIDS; i++) {
		if(asteroids[i].exists) {
			int size = ASTEROID_SIZE >> asteroids[i].size;
			asteroids[i].exist_timer++;
			asteroids[i].x += asteroids[i].vel_x;
			asteroids[i].y += asteroids[i].vel_y;
			WrapPos(size/2, size/2, &asteroids[i].x, &asteroids[i].y);
		}
	}
}

static void UnclearField()
{
	bool field_empty = true;
	for(int i=0; i<MAX_ASTEROIDS; i++) {
		if(asteroids[i].exists) {
			field_empty = false;
			break;
		}
	}
	if(field_empty) {
		ResetBullets();
		float s = cosf(ship.angle*M_DTOR);
		float c = sinf(ship.angle*M_DTOR);
		MakeAsteroid(0, -(s*ASTEROID_RESPAWN_RADIUS)+ship.x, (c*ASTEROID_RESPAWN_RADIUS)+ship.y, ASTEROID_VEL*s, -ASTEROID_VEL*c);
		MakeAsteroid(0, (s*ASTEROID_RESPAWN_RADIUS)+ship.x, -(c*ASTEROID_RESPAWN_RADIUS)+ship.y, ASTEROID_VEL*s, -ASTEROID_VEL*c);
	}
}

static void StateMain()
{
	UpdateShip();
	UpdateAsteroids();
	UpdateBullets();
	UnclearField();
}

static void PutSpriteCenter(N64Image *image, float x, float y, u32 color)
{
	ImagePutTint(image, x-(image->w/2), y-(image->h/2), color);
}

static void DrawShip()
{
	if(ship.invulnerable_timer < 6 || ship.invulnerable_timer & 0x2) {
		PutSpriteCenter(ship.image, ship.x, ship.y, GFX_COLOR_WHITE);
	}
}

static void DrawBullets()
{
	for(int i=0; i<MAX_BULLETS; i++) {
		if(bullets[i].exists) {
			//Place 1 pixel white rectangle
			GfxPutRect(bullets[i].x, bullets[i].y, 1, 1, GFX_COLOR_WHITE);
		}
	}
}

static void DrawAsteroids()
{
	for(int i=0; i<MAX_ASTEROIDS; i++) {
		if(asteroids[i].exists) {
			int frame = (asteroids[i].exist_timer/ASTEROID_ROT_DELAY) % ASTEROID_ROT_STEPS;
			PutSpriteCenter(asteroid_images[asteroids[i].size][frame], asteroids[i].x, asteroids[i].y, GFX_COLOR_YELLOW);
		}
	}
}

static void StateDraw()
{
	DrawShip();
	DrawBullets();
	DrawAsteroids();
}

static void DeleteAsteroidImages()
{
	for(int i=0; i<ASTEROID_NUM_SIZES; i++) {
		for(int j=0; j<ASTEROID_ROT_STEPS; j++) {
			ImageDelete(asteroid_images[i][j]);
			asteroid_images[i][j] = NULL;
		}
	}
}

static void StateDestroy()
{
	ImageDelete(ship.image);
	ship.image = NULL;
	DeleteAsteroidImages();
}

StateEntry Game_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};