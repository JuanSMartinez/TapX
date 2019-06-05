// CoreTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma warning(disable : 4996)
#include "pch.h"
#include <iostream>
#include "MotuPlayer.hpp"

void testSymbolCallback(TapX::TapsError err)
{
	printf("External callback used, played symbol with code %d\n", err);
}

void testSequenceCallback(TapX::TapsError err)
{
	printf("External callback for sequence used, played sequence with code %d\n", err);
}

void testStartFlagCallback(TapX::TapsError err)
{
	printf("External callback for start flag used, played flag with code %d\n", err);
}

int playSentenceStartFlag()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	player->registerSequencePlayedCallback(testSequenceCallback);
	if (player->successfulStart())
	{
		char buff[64];
		std::vector<std::string> result;
		std::vector<std::string>::iterator it;
		printf("Type a sentence to play or 'xx' to exit\n");
		fgets(buff, 64, stdin);
		if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
			buff[strlen(buff) - 1] = '\0';
		while (std::string(buff).compare("xx") != 0)
		{
			std::string typed(buff);

			player->playEnglishSentence(typed, 150, 1500, testStartFlagCallback, "KNOCK");

			printf("Type a sentence to play or 'xx' to exit\n");
			fgets(buff, 64, stdin);
			if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
				buff[strlen(buff) - 1] = '\0';
		}
		delete player;
		return 0;

	}
	return -1;
}

int playSentence()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	player->registerSequencePlayedCallback(testSequenceCallback);
	if (player->successfulStart())
	{
		char buff[64];
		std::vector<std::string> result;
		std::vector<std::string>::iterator it;
		printf("Type a sentence to play or 'xx' to exit\n");
		fgets(buff, 64, stdin);
		if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
			buff[strlen(buff) - 1] = '\0';
		while (std::string(buff).compare("xx") != 0)
		{
			std::string typed(buff);

			player->playEnglishSentence(typed, 150, 1500);

			printf("Type a sentence to play or 'xx' to exit\n");
			fgets(buff, 64, stdin);
			if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
				buff[strlen(buff) - 1] = '\0';
		}
		delete player;
		return 0;

	}
	return -1;
}


int getPhonemes()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	std::vector<std::string> result;
	if (player->successfulStart())
	{
		char buff[64];
		std::vector<std::string>::iterator it;
		printf("Type a sentence or 'xx' to exit\n");
		fgets(buff, 64, stdin);
		if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
			buff[strlen(buff) - 1] = '\0';
		while (std::string(buff).compare("xx") != 0)
		{
			std::string typed(buff);
			result.clear();
			player->getPhonemesOfSentence(&result, typed);

			it = result.begin();
			while (it != result.end())
			{
				printf("Phoneme: %s\n", (*it).c_str());
				it++;
			}


			printf("Type a sentence or 'xx' to exit\n");
			fgets(buff, 64, stdin);
			if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
				buff[strlen(buff) - 1] = '\0';
		}
		delete player;
		return 0;

	}
	return -1;
}

int getFlitePhonemes()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	if (player->successfulStart())
	{
		char buff[64];
		std::string result;
		printf("Type a sentence or 'xx' to exit\n");
		fgets(buff, 64, stdin);
		if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
			buff[strlen(buff) - 1] = '\0';
		while (std::string(buff).compare("xx") != 0)
		{
			std::string typed(buff);

			result = player->getRawFlitePhonemes(typed);
			printf("Result: -%s-\n", result.c_str());

			printf("Type a sentence or 'xx' to exit\n");
			fgets(buff, 64, stdin);
			if ((strlen(buff) > 0) && (buff[strlen(buff) - 1] == '\n'))
				buff[strlen(buff) - 1] = '\0';
		}
		delete player;
		return 0;

	}
	return -1;
}

int playSequenceOfSymbols()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	player->registerSequencePlayedCallback(testSequenceCallback);
	if (player->successfulStart())
	{
		char buff[64];
		printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
		scanf("%64s", buff);
		std::vector<std::string> vector;
		while (std::string(buff).compare("xx") != 0)
		{
			vector.clear();
			std::string typed(buff);

			size_t pos = 0;
			std::string token;
			std::string delimiter = ",";
			while ((pos = typed.find(delimiter)) != std::string::npos)
			{
				token = typed.substr(0, pos);
				vector.push_back(token);
				typed.erase(0, pos + delimiter.length());
			}
			int ici = (int)strtol(typed.c_str(), NULL, 10);

			printf("Playing sequence of symbols with %d ms ICI\n", ici);
			player->playSymbolSequence(vector, ici);

			printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
			scanf("%64s", buff);
		}
		delete player;
		return 0;

	}
	return -1;
}

int playSequenceOfSymbolsStartFlag()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	player->registerSequencePlayedCallback(testSequenceCallback);
	if (player->successfulStart())
	{
		char buff[64];
		printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
		scanf("%64s", buff);
		std::vector<std::string> vector;
		while (std::string(buff).compare("xx") != 0)
		{
			vector.clear();
			std::string typed(buff);

			size_t pos = 0;
			std::string token;
			std::string delimiter = ",";
			while ((pos = typed.find(delimiter)) != std::string::npos)
			{
				token = typed.substr(0, pos);
				vector.push_back(token);
				typed.erase(0, pos + delimiter.length());
			}
			int ici = (int)strtol(typed.c_str(), NULL, 10);

			printf("Playing sequence of symbols with %d ms ICI\n", ici);
			player->playSymbolSequence(vector, ici, testStartFlagCallback, "KNOCK");

			printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
			scanf("%64s", buff);
		}
		delete player;
		return 0;

	}
	return -1;
}

int playIndividualSymbols()
{
	TapX::MotuPlayer* player = TapX::MotuPlayer::getInstance();
	printf("Player created\n");
	player->startSession();
	player->registerSymbolPlayedCallback(testSymbolCallback);
	if (player->successfulStart())
	{
		char buff[5];
		printf("Type a symbol label or 'xx' to exit\n");
		scanf("%5s", buff);
		while (std::string(buff).compare("xx") != 0)
		{
			player->playHapticSymbol(std::string(buff));

			printf("Type a phoneme label to play or 'xx' to exit\n");
			scanf("%5s", buff);
		}
		delete player;
		return 0;
	}
	printf("Did not start\n");
	return -1;
}

int main()
{
	//return playIndividualSymbols();
	//return playSequenceOfSymbols();
	//return getFlitePhonemes();
	//return getPhonemes();
	//return playSentence();
	//return playSequenceOfSymbolsStartFlag();
	return playSentenceStartFlag();
	
}

