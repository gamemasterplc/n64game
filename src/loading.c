#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "save.h"

#define LOADING_DELAY 15

static N64Image *loading;
static int timer;

static void StateInit()
{
	loading = ImageLoad("/gfx/loading.i8.img");
	timer = LOADING_DELAY;
}

static void StateMain()
{
	if(--timer == 0) {
		StateSetNext(STATE_GAME);
	}
}

static void StateDraw()
{
	ImagePut(loading, (GfxGetWidth()-loading->w)/2, (GfxGetHeight()-loading->h)/2);
}

static void StateDestroy()
{
	ImageDelete(loading);
	loading = NULL;
}

StateEntry Loading_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};