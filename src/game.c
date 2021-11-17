#include "state.h"
#include "gfx.h"
#include "image.h"

static N64Image *large_sprite;

static void StateInit()
{
	large_sprite = ImageLoad("/gfx/large_sprite.rgba16.img");
}

static void StateMain()
{
	
}

static void StateDraw()
{
	int screen_w = GfxGetWidth();
	int screen_h = GfxGetHeight();
	for(int i=0; i<(screen_w+63)/64; i++) {
		for(int j=0; j<(screen_h+63)/64; j++) {
			ImagePut(large_sprite, i*64, j*64);
		}
	}
}

static void StateDestroy()
{
	ImageDelete(large_sprite);
}

StateEntry Game_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};