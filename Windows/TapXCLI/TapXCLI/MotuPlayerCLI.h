#pragma once
#include "../../../inc/MotuPlayer.hpp"
using namespace System;
using namespace TapX;

namespace TapXCLI 
{
	
	public delegate void SymbolCallback(int error);
	delegate void SymbolPlayedCallbackCLI(TapX::TapsError error);

	public ref class MotuPlayerCLI
	{

	private:

		//Internal player instance
		MotuPlayer* playerInstance;

		SymbolCallback^ external;

		void InternalCallback(TapX::TapsError err);
		

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
		
		void RegisterExternalCallback(SymbolCallback^ callback);


	};
}
