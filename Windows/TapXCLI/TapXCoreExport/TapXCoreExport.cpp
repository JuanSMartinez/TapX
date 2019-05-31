// TapXCoreExport.cpp : Defines the exported functions for the DLL application.
//

#include "TapXCoreExport.h"

DLLEXPORT void createMotuPlayer()
{
	player = TapX::MotuPlayer::getInstance();
	player->startSession();
}

DLLEXPORT void finalize()
{
	if (player != 0)
	{
		delete player;
	}
}

DLLEXPORT void playHapticSymbol(const char* symbol)
{
	if (player != 0 && player->successfulStart())
	{
		std::string code(symbol);
		player->playHapticSymbol(code);
	}
}

DLLEXPORT void playSequence(const char** sequence, int sequenceLength, int ici)
{
	if (player != 0 && player->successfulStart())
	{
		std::vector<std::string> vect;
		for (int i = 0; i < sequenceLength; i++)
		{
			std::string str(sequence[i]);
			vect.push_back(str);
		}
		player->playSymbolSequence(vect, ici);
	}
}

DLLEXPORT void playEnglishSentence(const char* sentence, int ici, int iwi)
{
	if (player != 0 && player->successfulStart())
	{
		std::string str(sentence);
		player->playEnglishSentence(sentence, ici, iwi);
	}
}
