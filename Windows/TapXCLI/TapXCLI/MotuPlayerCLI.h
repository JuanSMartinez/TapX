#pragma once
#include "../../../inc/MotuPlayer.hpp"
using namespace System;
using namespace TapX;

namespace TapXCLI {
	public ref class MotuPlayerCLI
	{
	protected:
		MotuPlayer* playerInstance;
	public:

		//Constructor
		MotuPlayerCLI();

		//Destructor
		~MotuPlayerCLI();

		//Finalizer
		!MotuPlayerCLI();

		//Return the instance of the underlying player
		MotuPlayer* GetInstance();

		//Did the session start correctly
		bool SessionStarted();

	};
}
