#include <nusys.h>
#include "bool.h"
#include "gfx.h"
#include "save.h"

static u8 save_data[8];
static bool used_save;

static u8 GetSaveChecksum()
{
	return save_data[0];
}

static u8 CalcSaveChecksum()
{
	u8 sum = 0;
	for(int i=0; i<7; i++) {
		sum += save_data[i+1];
	}
	return sum;
}

static u16 GetSaveMagic()
{
	return (save_data[1] << 8)|save_data[2];
}

void SaveSetWide(bool flag)
{
	save_data[3] = flag;
	if(flag) {
		GfxSetScreenSize(424, 240);
	} else {
		GfxSetScreenSize(320, 240);
	}
}

void SaveToggleWide()
{
	save_data[3] ^= 1;
	if(save_data[3]) {
		GfxSetScreenSize(424, 240);
	} else {
		GfxSetScreenSize(320, 240);
	}
}

bool SaveGetWide()
{
	return save_data[3];
}

void SaveSetHighScore(u32 score)
{
	save_data[4] = score >> 24;
	save_data[5] = (score >> 16) & 0xFF;
	save_data[6] = (score >> 8) & 0xFF;
	save_data[7] = score & 0xFF;
}

u32 SaveGetHighScore()
{
	u32 score;
	score = save_data[4] << 24;
	score |= save_data[5] << 16;
	score |= save_data[6] << 8;
	score |= save_data[7];
	return score;
}

void SaveReset()
{
	save_data[1] = 0x53;
	save_data[2] = 0x56;
	SaveSetWide(false);
	SaveSetHighScore(0);
	save_data[0] = CalcSaveChecksum();
}

void SaveInit()
{
	nuEepromMgrInit();
	SaveReset();
	if(nuEepromCheck() == EEPROM_TYPE_4K || nuEepromCheck() == EEPROM_TYPE_16K) {
		used_save = true;
		nuEepromRead(0, save_data, 8);
		if(GetSaveMagic() != 0x5356 || save_data[0] != CalcSaveChecksum()) {
			SaveReset();
		}
	}
}

void SaveUpdate()
{
	save_data[0] = CalcSaveChecksum();
	if(used_save) {
		nuEepromWrite(0, save_data, 8);
	}
}