#pragma once
#include "../../../inc/MotuPlayer.hpp"
#include <msclr/marshal_cppstd.h>
using namespace System;
using namespace TapX;

namespace TapXCLI 
{
	//CLI delegates for playback
	public delegate void SymbolCallback(int error);
	public delegate void SequenceCallback(int error);

	//Internal delegates for playback
	delegate void SymbolPlayedCallbackCLI(TapX::TapsError error);
	delegate void SequencePlayedCallbackCLI(TapX::TapsError error);

	public ref class MotuPlayerCLI
	{

	private:

		//Internal player instance
		MotuPlayer* playerInstance;

		//External callback for symbol playback
		SymbolCallback^ externalSymbolCallback;

		//Internal callback for symbol playback
		void InternalSymbolCallback(TapX::TapsError err);

		//External callback for sequence playback 
		SequenceCallback^ externalSequenceCallback;

		//Internal callback for sequenc playback
		void InternalSequenceCallback(TapX::TapsError err);
		

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
		
		//Register an external symbol played callback
		void RegisterExternalSymbolCallback(SymbolCallback^ callback);

		//Register an external sequence played callback
		void RegisterExternalSequenceCallback(SequenceCallback^ callback);

		//Play a sequence of symbols with a defined ICI
		void PlaySequenceOfSymbols(array<String^>^ sequence, int ICI);

		//Play an English sentence using Flite with a defined ICI and IWI
		void PlayEnglishSentence(String^ sentence, int ICI, int IWI);


	};
}
