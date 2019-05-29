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

		//Did the session start correctly
		bool SessionStarted();

		//Play a haptic symbol 
		void PlayHapticSymbol(String^ code);

	};
}
