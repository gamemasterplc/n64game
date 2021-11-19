#include <ultra64.h>
#include "state.h"
#include "gfx.h"
#include "image.h"
#include "pad.h"
#include "title.h"
#include "save.h"

static bool put_loading;

static N64Image *start_text;
static N64Image *logo;

static void StateInit()
{
	if(PadIsConnected(0)) {
		start_text = ImageLoad("/gfx/press_start.i8.img");
	} else {
		start_text = ImageLoad("/gfx/no_controller.i8.img");
	}
	logo = ImageLoad("/gfx/logo.i8.img");
}

static void StateMain()
{
	if(PadGetPressedButtons(0) & START_BUTTON) {
		if(PadGetHeldButtons(0) & Z_TRIG) {
			SaveToggleWide();
			SaveUpdate();
		}
		StateSetNext(STATE_LOADING);
	}
	if(PadGetHeldButtons(0) & L_TRIG) {
		if(PadGetPressedButtons(0) & R_TRIG) {
			SaveReset();
			SaveUpdate();
		}
	}
}

static void StateDraw()
{
	ImagePut(logo, (GfxGetWidth()-logo->w)/2, MARGIN_Y);
	ImagePut(start_text, (GfxGetWidth()-start_text->w)/2, GfxGetHeight()-MARGIN_Y-start_text->h);
}

static void StateDestroy()
{
	ImageDelete(start_text);
	start_text = NULL;
	ImageDelete(logo);
	logo = NULL;
}

StateEntry Title_StateData = {
	StateInit, StateMain, StateDraw, StateDestroy
};