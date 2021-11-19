#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "save.h"

static N64Image *loading;

static void StateInit()
{
	loading = ImageLoad("/gfx/loading.i8.img");
}

static void StateMain()
{
	StateSetNext(STATE_GAME);
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