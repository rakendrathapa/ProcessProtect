#pragma once

#define DRIVER_PREFIX "ProcessProtect: "

#define PROCESS_TERMINATE	1		// Needs to defined explictly. In WDK headers (only PROCESS_ALL_ACCESS is defined. 

#include "FastMutex.h"

constexpr int MaxPids = 256;

struct Globals
{
	int PidsCount;	// currently protected process count
	ULONG Pids[MaxPids];	// protected PIDs
	FastMutex Lock;
	PVOID RegHandle;

	void Init()
	{
		Lock.Init();
	}
};