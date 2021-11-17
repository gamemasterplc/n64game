#include "pad.h"

static u8 connected_pads;
static NUContData pad_data[NU_CONT_MAXCONTROLLERS];

void PadInit()
{
	connected_pads = nuContInit();
}

void PadUpdate()
{
	nuContDataGetExAll(pad_data);
}

bool PadIsConnected(int pad_num)
{
	return (connected_pads & (1 << pad_num)) != 0;
}

u16 PadGetHeldButtons(int pad_num)
{
	return pad_data[pad_num].button;
}

s8 PadGetStickX(int pad_num)
{
	return pad_data[pad_num].stick_x;
}

s8 PadGetStickY(int pad_num)
{
	return pad_data[pad_num].stick_y;
}

u16 PadGetPressedButtons(int pad_num)
{
	return pad_data[pad_num].trigger;
}