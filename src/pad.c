#include <nusys.h>
#include "bool.h"

static u8 connected_pads;
NUContData pad_data[NU_CONT_MAXCONTROLLERS];

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
	return connected_pads & (1 << pad_num);
}