// TapXCoreExport.cpp : Defines the exported functions for the DLL application.
//

#include "TapXCoreExport.h"

//External callbacks
SymbolCallback externalSymbolCallbak = 0;
SequenceCallback externalSequenceCallback = 0;

//Internal callbacks
void internalSymbolCallback(TapX::TapsError err)
{
	if (externalSymbolCallbak != 0)
		externalSymbolCallbak((int)err);
}

void internalSequenceCallback(TapX::TapsError err)
{
	if (externalSequenceCallback != 0)
		externalSequenceCallback((int)err);
}

DLLEXPORT void createMotuPlayer()
{
	player = TapX::MotuPlayer::getInstance();
	player->startSession();
	if (player->successfulStart())
	{
		player->registerSymbolPlayedCallback(internalSymbolCallback);
		player->registerSequencePlayedCallback(internalSequenceCallback);
	}
}

DLLEXPORT void finalize()
{
	if (player != 0)
	{
		delete player;
		player = 0;
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

DLLEXPORT void registerExternalSymbolCallback(SymbolCallback callback)
{
	externalSymbolCallbak = callback;
}

DLLEXPORT void registerExternalSequenceCallback(SequenceCallback callback)
{
	externalSequenceCallback = callback;
}
