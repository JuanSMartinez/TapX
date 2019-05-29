#include "MotuPlayerCLI.h"

using namespace System::Runtime::InteropServices;

namespace TapXCLI
{

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

	//Did the session start correctly
	bool MotuPlayerCLI::SessionStarted()
	{
		return playerInstance->successfulStart();
	}

	//Play a haptic symbol 
	void MotuPlayerCLI::PlayHapticSymbol(String^ code)
	{
		std::string str_code((const char*)Marshal::StringToHGlobalAnsi(code).ToPointer());
		playerInstance->playHapticSymbol(str_code);
	}
}