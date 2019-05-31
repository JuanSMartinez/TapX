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
	
	//Play a haptic symbol
	DLLEXPORT void playHapticSymbol(const char* symbol);

	//Play a sequence of symbols with an ICI
	DLLEXPORT void playSequence(const char** sequence, int sequenceLength, int ici);

	//Play an English sentence
	DLLEXPORT void playEnglishSentence(const char* sentence, int ici, int iwi);

}

