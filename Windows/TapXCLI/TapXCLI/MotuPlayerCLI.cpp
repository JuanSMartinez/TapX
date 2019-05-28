#include "MotuPlayerCLI.h"

using namespace TapXCLI;

//Constructor
MotuPlayerCLI::MotuPlayerCLI()
{
	playerInstance = MotuPlayer::getInstance();
	playerInstance->startSession();
}

//Destructor
MotuPlayerCLI::~MotuPlayerCLI()
{
	if (playerInstance != nullptr)
		delete playerInstance;
}

//Finalizer
MotuPlayerCLI::!MotuPlayerCLI()
{
	if (playerInstance != nullptr)
		delete playerInstance;
}

//Get the underlying instance of the player
MotuPlayer*  MotuPlayerCLI::GetInstance()
{
	return playerInstance;
}

//Did the session start correctly
bool MotuPlayerCLI::SessionStarted()
{
	return playerInstance->successfulStart();
}