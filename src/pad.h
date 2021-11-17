#pragma once

#include <ultra64.h>
#include "bool.h"

void PadInit();
void PadUpdate();
bool PadIsConnected(int pad_num);
u16 PadGetHeldButtons(int pad_num);
s8 PadGetStickX(int pad_num);
s8 PadGetStickY(int pad_num);
u16 PadGetPressedButtons(int pad_num);