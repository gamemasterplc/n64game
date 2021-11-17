#pragma once

#include <ultra64.h>

typedef void (*StateFunc)();

typedef struct state_entry {
	StateFunc init;
	StateFunc main;
	StateFunc draw;
	StateFunc destroy; //May be NULL
} StateEntry;

#define DEFINE_STATE(enum, _1) enum,

//Define state indices
typedef enum state_index {
	#include "state_table.h"
} StateIndex;

#undef DEFINE_STATE

void SetNextState(StateIndex state);