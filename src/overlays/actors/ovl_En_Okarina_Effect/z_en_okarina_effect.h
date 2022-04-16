#pragma once
#define Z_EN_OKARINA_EFFECT_H

#include "global.h"
#include "ultra64.h"

struct EnOkarinaEffect;

typedef void (*EnOkarinaEffectActionFunc)(struct EnOkarinaEffect*, GlobalContext*);

struct EnOkarinaEffect
{
	/* 0x0000 */ Actor actor;
	/* 0x014C */ TimerU16 timer;
	/* 0x0150 */ EnOkarinaEffectActionFunc actionFunc;
};
