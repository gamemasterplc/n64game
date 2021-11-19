#pragma once

#include <PR/ultratypes.h>
#include "bool.h"

void SaveSetWide(bool flag);
void SaveToggleWide();
bool SaveGetWide();
void SaveSetHighScore(u32 score);
u32 SaveGetHighScore();
void SaveReset();
void SaveInit();
void SaveUpdate();
