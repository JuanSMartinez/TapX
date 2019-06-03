#pragma once
#include "../../../inc/MotuPlayer.hpp"
#define DLLEXPORT _declspec(dllexport)

//Player instance
TapX::MotuPlayer* player = 0;

//External callback definitions
typedef void(__stdcall * SymbolCallback)(int error);
typedef void(__stdcall * SequenceCallback)(int error);

extern "C"
{
	//Create Motu player instance and start the session
	DLLEXPORT void createMotuPlayer();

	//Finalize 
	DLLEXPORT void finalize();
	
	//Play a haptic symbol
	DLLEXPORT void playHapticSymbol(const char* symbol);

	//Play a sequence of symbols with an ICI
	DLLEXPORT void playSequence(const char** sequence, int sequenceLength, int ici);

	//Play an English sentence
	DLLEXPORT void playEnglishSentence(const char* sentence, int ici, int iwi);

	//Register an external symbol played callback
	DLLEXPORT void registerExternalSymbolCallback(SymbolCallback callback);

	//Register an external sequence played callback
	DLLEXPORT void registerExternalSequenceCallback(SequenceCallback callback);

}

