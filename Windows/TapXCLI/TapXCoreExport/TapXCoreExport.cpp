// TapXCoreExport.cpp : Defines the exported functions for the DLL application.
//

#include "TapXCoreExport.h"

DLLEXPORT void createMotuPlayer()
{
	player = TapX::MotuPlayer::getInstance();
	player->startSession();
}

DLLEXPORT void finalize()
{
	if (player != 0)
	{
		delete player;
	}
}
