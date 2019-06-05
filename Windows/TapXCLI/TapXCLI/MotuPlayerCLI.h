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
	public delegate void StartFlagCallback(int error);

	//Internal delegates for playback
	delegate void SymbolPlayedCallbackCLI(TapX::TapsError error);
	delegate void SequencePlayedCallbackCLI(TapX::TapsError error);
	delegate void StartFlagPlayedCallbackCLI(TapX::TapsError error);

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

		//Internal callback for sequence playback
		void InternalSequenceCallback(TapX::TapsError err);

		//External callback for startFlag playback 
		StartFlagCallback^ externalStartFlagCallback;

		//Internal callback for start flag
		void InternalStartFlagCallback(TapX::TapsError err);
		

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

		//Play a sequence of symbols with a defined ICI, a start flag and a start flag callback
		void PlaySequenceOfSymbols(array<String^>^ sequence, int ICI, StartFlagCallback^ startFlagCallback, String^ startFlag);

		//Play an English sentence using Flite with a defined ICI, IWI, a start flag and a start flag callback
		void PlayEnglishSentence(String^ sentence, int ICI, int IWI, StartFlagCallback^ startFlagCallback, String^ startFlag);

		//Get raw flite phonemes of a sentence
		String^ GetFlitePhonemesOf(String^ sentence);

		//Get TAPS phonemes of a sentence
		array<String^>^ GetPhonemesOf(String^ sentence);


	};
}
