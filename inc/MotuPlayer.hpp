#pragma once
#include "HapticSymbol.hpp"
#include <unordered_map>
#ifdef __linux__
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <portaudio.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <strsafe.h>
//#include <thread>
//#include <mutex>
#include "../inc/portaudio.h"
#endif

#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <atomic>

#ifdef _WIN32
#define PHONEMES_PATH ("\\Symbols\\Phonemes\\")
#define CHUNKS_PATH ("\\Symbols\\Chunks\\")
#define FLAGS_PATH ("\\Symbols\\Flags\\")
#define FLITE_MAP_PATH ("\\Symbols\\Flite\\mapping.csv")
#else
#define PHONEMES_PATH ("./Symbols/Phonemes/")
#define CHUNKS_PATH ("./Symbols/Chunks/")
#define FLAGS_PATH ("./Symbols/Flags/")
#define FLITE_MAP_PATH ("./Symbols/Flite/mapping.csv")
#endif



#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (64)

namespace TapX
{
    //Error types
    enum TapsError
    {
        TapsNoError, //0
        TapsNoMotuFound, //1
        TapsErrorInitializing, //2
        TapsErrorStartingStream, //3
        TapsErrorOpeningStream, //4
        TapsErrorSettingFinishedStreamCallback, //5
        TapsErrorClosingStream, //6
        TapsErrorStoppingStream, //7
        TapsErrorSymbolNotFound, //8
        TapsErrorNoPlaybackSession, //9
        TapsErrorSignalAlreadyPlaying //10
    };

    //Types of callbacks that can be registered
#ifdef _WIN32
    typedef void (__stdcall *SymbolPlayedCallback)(TapsError);
    typedef void (__stdcall *SequencePlayedCallback)(TapsError);
	typedef void(__stdcall *StartFlagPlayedCallback)(TapsError);
	typedef void(__stdcall *SentenceTranscribedCallback)(void);
#else
    typedef void (*SymbolPlayedCallback)(TapsError);
    typedef void (*SequencePlayedCallback)(TapsError);
	typedef void (*StartFlagPlayedCallback)(TapsError);
	typedef void (*SentenceTranscribedCallback)(void);
#endif

    //Structure for a sequence of symbols
    typedef struct 
    {
        int ici;
        int iwi;
        TapsError err;
		std::vector<std::string> sequence;
		StartFlagPlayedCallback startFlagCallback;
		std::string startFlag;

		//If the sequence represents an English sentence
		SentenceTranscribedCallback sentenceTranscribedCallback;
		std::string sentence;

    }SequenceStructure;


    class MotuPlayer
    {
        private:
            static MotuPlayer* instance ;
            std::unordered_map<std::string, HapticSymbol*> *phonemes;
            std::unordered_map<std::string, HapticSymbol*> *chunks;
            std::unordered_map<std::string, HapticSymbol*> *flags;
            std::unordered_map<std::string, std::string> fliteMapping;
			HapticSymbol* iciSilence;
			HapticSymbol* iwiSilence;
            int numberOfChunks, numberOfPhonemes, numberOfFlags, lastICI, lastIWI;
            bool sessionStarted, playing;
            SymbolPlayedCallback symbolCallback;
            SequencePlayedCallback sequenceCallback;
            HapticSymbol* currentPlayingSymbol;
            PaStream* stream;
            float *zeros;

            MotuPlayer();
            void initializeData();
            int intializeMap(std::string path, std::unordered_map<std::string, HapticSymbol*> *map);
            void initializeFliteMapping();
            PaDeviceIndex getMotuIndex();
            HapticSymbol* getCurrentPlayingSymbol();
            void signalSymbolCallback(TapsError err);
            TapsError playSymbol(std::string code);
            void playSequence(std::vector<std::string> sequence, int ici, int iwi, StartFlagPlayedCallback startFlagCallback, std::string startFlag);
            void splitStringBy(std::vector<std::string>* result, std::string delimiter, std::string str);
			TapsError startPlaybackSession();
			TapsError stopPlaybackSession();
            
            static int paCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);
            static void streamFinishedCallback(void* userData);
            
        public:
            ~MotuPlayer();
            static MotuPlayer* getInstance();
            float* getZeros();
            void startSession();
            bool successfulStart();
            void registerSymbolPlayedCallback(SymbolPlayedCallback callback);
            void registerSequencePlayedCallback(SequencePlayedCallback callback);
            SymbolPlayedCallback getRegisteredSymbolCallback();
            bool isPlaying();
            void signalSentencePlayedCallback(TapsError err);
			void interruptStream();
			void startStream();
			bool isStreamActive();
			void setSilenceParameters(int ici, int iwi);
			void playICI();
			void playIWI();

			//Playing functions
            TapsError playHapticSymbol(std::string code);
            void playSymbolSequence(std::vector<std::string> sequence, int ici);
			void playSymbolSequence(std::vector<std::string> sequence, int ici, StartFlagPlayedCallback startFlagCallback, std::string startFlag);

			//Flite functions
            std::string getRawFlitePhonemes(std::string sentence);
            void getPhonemesOfSentence(std::vector<std::string>* result, std::string sentence);

			//Play sentence using flite
			void playEnglishSentence(std::string sentence, int ici, int iwi);
            void playEnglishSentence(std::string sentence, int ici, int iwi, StartFlagPlayedCallback startFlagCallback, std::string startFlag, SentenceTranscribedCallback sentenceTranscribedCallback);
       
    };
}