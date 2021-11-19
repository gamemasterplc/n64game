#include "gfx.h"
#include "fs.h"
#include "state.h"
#include "pad.h"
#include "save.h"

#define DEFINE_STATE(_0, name) extern StateEntry name##_StateData;
#include "state_table.h"
#undef DEFINE_STATE

#define DEFINE_STATE(_0, name) &name##_StateData,
static StateEntry *states[] = {
	#include "state_table.h"
};
#undef DEFINE_STATE

static volatile StateIndex curr_state;
static volatile StateIndex next_state;

void StateSetNext(StateIndex state)
{
	next_state = state;
}

void mainproc()
{
	GfxInit();
	PadInit();
	SaveInit();
	FSInit();
	next_state = STATE_TITLE;
	if(SaveGetWide()) {
		GfxSetScreenSize(424, 240);
	}
	while(1) {
		curr_state = next_state;
		StateEntry *state = states[curr_state];
		state->init();
		while(next_state == curr_state) {
			PadUpdate();
			state->main();
			GfxStartFrame();
			state->draw();
			GfxEndFrame();
		}
		if(state->destroy) {
			state->destroy();
		}
	}
}