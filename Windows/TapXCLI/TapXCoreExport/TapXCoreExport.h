#pragma once
#include "../../../inc/MotuPlayer.hpp"
#define DLLEXPORT _declspec(dllexport)

//Player instance
TapX::MotuPlayer* player = 0;

extern "C"
{
	//Create Motu player instance and start the session
	DLLEXPORT void createMotuPlayer();

	//Finalize 
	DLLEXPORT void finalize();

}

