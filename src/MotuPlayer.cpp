#include "../inc/MotuPlayer.hpp"

namespace TapX
{

    //Mutex
#ifdef __linux__
    pthread_mutex_t motu_lock;
#else
	bool motu_lock = false;
#endif

#ifdef __linux__
	pthread_mutex_t lock;
#else
	CRITICAL_SECTION lock;
#endif

	//Synchronization for playing sequences
	std::atomic<bool> playingSequenceLock(false);

	//Mutex to control threads that play sequences
	HANDLE sequenceMutex;

    //Structure of a sequence of symbols
    SequenceStructure* sequenceStruct;

    //Previously set callback for playing symbols
    SymbolPlayedCallback previousSymbolCallback = 0;

    //Is motu playing
#ifdef __linux__
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
#else
	//std::condition_variable condition;
	CONDITION_VARIABLE condition;
#endif

    //NULL instance
    MotuPlayer* MotuPlayer::instance = 0;

    //Get the singleton instance
    MotuPlayer* MotuPlayer::getInstance()
    {
        if(instance == 0)
        {
            instance = new MotuPlayer();
        }
        return instance;
    }

    //Destructor
    MotuPlayer::~MotuPlayer()
    {
		stopPlaybackSession();
        free(zeros);
		phonemes->clear();
		chunks->clear();
		flags->clear();
		delete phonemes;
		delete chunks;
		delete flags;
#ifdef _WIN32
		CloseHandle(sequenceMutex);
		DeleteCriticalSection(&lock);
#endif
#ifdef linux
        pthread_mutex_destroy(&motu_lock);
#endif

    }

    //Default constructor
    MotuPlayer::MotuPlayer()
    {
        //Data initialization
        currentPlayingSymbol = 0;
        sessionStarted = false;
        playing = false;
        symbolCallback = 0;
        sequenceCallback = 0;
        zeros = (float*)calloc(24 * FRAMES_PER_BUFFER, sizeof(float));
		phonemes = new std::unordered_map<std::string, HapticSymbol*>;
		flags = new std::unordered_map<std::string, HapticSymbol*>;
		chunks = new std::unordered_map<std::string, HapticSymbol*>;
#ifdef _WIN32
		sequenceMutex = CreateMutex(NULL, FALSE, NULL);
		InitializeConditionVariable(&condition);
		InitializeCriticalSection(&lock);
#endif
        initializeData();
            
    };

    //Get the zeros vector
    float* MotuPlayer::getZeros()
    {
        return zeros;
    }

    //Start the playback session
    void MotuPlayer::startSession()
    {
        //Try to start the playback session
		if (!sessionStarted)
		{
			TapsError err = startPlaybackSession();
			sessionStarted = err == TapsNoError;
			if (sessionStarted)
				printf("Playback session started successfuly\n");
			else
			{
				printf("ERROR: Could not start playback session, error code: %d\n", err);
			}
		}
    }

    //Get files on a given path
    int MotuPlayer::intializeMap(std::string path, std::unordered_map<std::string, HapticSymbol*> *map)
    {
		std::string csv_suffix = ".csv";
		int numberOfSymbols = 0;

#ifdef linux
        DIR *dp;
        struct dirent *dirp;
        dp = opendir(path.c_str());
        if( dp  == NULL)
        {
            printf("Error %d, could not open the folder %s\n", errno, path.c_str());
            return 0;
        }

        while((dirp = readdir(dp))!= NULL)
        {
            //Only the csv files 
            std::string file_name = dirp->d_name;
            if(file_name.size() >= csv_suffix.size() &&
			    file_name.compare(file_name.size() - csv_suffix.size(), csv_suffix.size(), csv_suffix) == 0)
                {
                    std::string symbolName = file_name.substr(0, file_name.size()-4);
                    HapticSymbol* symbol = new HapticSymbol(symbolName);
                    symbol->initializeData(path);
                    std::pair<std::string, HapticSymbol*> newPair (symbolName, symbol);
                    map.insert(newPair);
                    numberOfSymbols++;
                }
                
        }
        closedir(dp);
        
#endif
#ifdef _WIN32
		std::string basePath = "";
		std::vector<wchar_t> pathBuf;
		DWORD copied = 0;
		do {
			pathBuf.resize(pathBuf.size() + MAX_PATH);
			copied = (DWORD)GetModuleFileNameW(0, &pathBuf.at(0), pathBuf.size());
		} while (copied >= pathBuf.size());

		pathBuf.resize(copied);
		std::string fullPath(pathBuf.begin(), pathBuf.end());
		std::string::size_type pos = std::string(fullPath).find_last_of("\\/");
		basePath = std::string(fullPath).substr(0, pos) + path;


		DWORD dwError = 0;
		WIN32_FIND_DATA ffd;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;

		StringCchCopy(szDir, MAX_PATH, basePath.c_str());
		StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

		// Find the first file in the directory.

		hFind = FindFirstFile(szDir, &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			printf("No files on path\n");
			return 0;
		}

		// For all the csv files in the directory 
		do
		{
			std::string file_name(ffd.cFileName);
			if (file_name.size() >= csv_suffix.size() &&
				file_name.compare(file_name.size() - csv_suffix.size(), csv_suffix.size(), csv_suffix) == 0)
			{
				std::string symbolName = file_name.substr(0, file_name.size() - 4);
				HapticSymbol* symbol = new HapticSymbol(symbolName);
				symbol->initializeData(basePath);
				std::pair<std::string, HapticSymbol*> newPair(symbolName, symbol);
				map->insert(newPair);
				numberOfSymbols++;
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			printf("Error closing the file reading strategy (Windows)\n");
			return 0;
		}
		FindClose(hFind);
#endif

		return numberOfSymbols;
    }

    //Initialize dictionary with the flite mapping
    void MotuPlayer::initializeFliteMapping()
    {
        std::string line;
        std::ifstream file;
#ifdef __linux__
        file.open(FLITE_MAP_PATH);
#else
		std::string basePath = "";
		std::vector<wchar_t> pathBuf;
		DWORD copied = 0;
		do {
			pathBuf.resize(pathBuf.size() + MAX_PATH);
			copied = (DWORD)GetModuleFileNameW(0, &pathBuf.at(0), pathBuf.size());
		} while (copied >= pathBuf.size());

		pathBuf.resize(copied);
		std::string fullPath(pathBuf.begin(), pathBuf.end());
		std::string::size_type pos = std::string(fullPath).find_last_of("\\/");
		basePath = std::string(fullPath).substr(0, pos) + FLITE_MAP_PATH;
		file.open(basePath);
#endif
        std::string delimiter = ",";
        std::string first;
        std::string second;
        size_t pos_f = 0;
        while(file.good())
        {
            getline(file,line);
			pos_f = line.find(delimiter);
            first = line.substr(0, pos_f);
            second = line.substr(pos_f +1, line.size());
            
            std::pair<std::string, std::string> newPair (first, second);
            fliteMapping.insert(newPair);
        }
        file.close();
    }


    //Initialize all data
    void MotuPlayer::initializeData()
    {
        //Phonemes
        numberOfPhonemes = intializeMap(PHONEMES_PATH, phonemes);
        if(numberOfPhonemes == 0)
        {
            printf("WARNING: Could not initialize phonemes or the phonemes folder is empty\n");
        }

        //Chunks
        numberOfChunks = intializeMap(CHUNKS_PATH, chunks);
        if(numberOfChunks == 0)
        {
            printf("WARNING: Could not initialize chunks or the chunks folder is empty\n");
        }

        //Flags
        numberOfFlags = intializeMap(FLAGS_PATH, flags);
        if(numberOfFlags == 0)
        {
            printf("WARNING: Could not initialize flags or the flags folder is empty\n");
        }

        //Flite mapping
        initializeFliteMapping();

		//Periods of silence
		iciSilence = new HapticSymbol("ICI");
		iciSilence->initializeData(100, SAMPLE_RATE);
		lastICI = 100;
		iwiSilence = new HapticSymbol("IWI");
		iwiSilence->initializeData(100, SAMPLE_RATE);
		lastIWI = 100;

    }

    //Get the current playing symbol
    HapticSymbol* MotuPlayer::getCurrentPlayingSymbol()
    {
        return currentPlayingSymbol;
    }

    //Portaudio callback
    int MotuPlayer::paCallback(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
    {      
        float *out = (float*)outputBuffer;
        MotuPlayer *player = (MotuPlayer*)userData;
        HapticSymbol* symbol = player->getCurrentPlayingSymbol();
        (void) timeInfo; /* Prevent unused variable warnings. */
        (void) statusFlags;
        (void) inputBuffer;
        if(symbol != 0)
        {
            unsigned long rowsRemaining = (unsigned long)symbol->remainingRows();
            if( rowsRemaining <= framesPerBuffer)
            {
                //Copy the remaining samples into the buffer
                memcpy(out, symbol->samplesFromRow((int)symbol->getMatrixRowIndex()), sizeof(float)*rowsRemaining*24);

                //Fill the rest of the buffer with zeros
                out += 24*rowsRemaining;
                memcpy(out, player->getZeros(), sizeof(float)*24*(framesPerBuffer-rowsRemaining));
                player->currentPlayingSymbol = 0;
				player->signalSymbolCallback(TapsNoError);
            }
            else
            {
                //Fill all the frames of the buffer with data from matrix and increase the index by the frames consumed
                memcpy(out, symbol->samplesFromRow((int)symbol->getMatrixRowIndex()), sizeof(float)*framesPerBuffer*24);
                symbol->increaseIndexBy(framesPerBuffer);
				
            }
	
        }
        else
        {
			memcpy(out, player->getZeros(), sizeof(float)*24*framesPerBuffer);
        }
		return paContinue;

        
    }

    //Stream finished callback
    void MotuPlayer::streamFinishedCallback(void* userData)
    {
        MotuPlayer *player = (MotuPlayer*)userData;
        printf("Stream finished\n");
    }

    //Find the motu index
    PaDeviceIndex MotuPlayer::getMotuIndex(){
        int numDevices = Pa_GetDeviceCount();
        const PaDeviceInfo *deviceInfo;
        PaDeviceIndex motu = paNoDevice;

#ifdef __linux__
		const char* substr = "24Ao";
#endif

#ifdef _WIN32
		std::string motuName("Out 1-24 (Out 1-24)");
		std::string hostName("Windows WASAPI");
		const PaHostApiInfo *hostInfo;
#endif
        for(int i = 0; i < numDevices && motu == paNoDevice; i++)
		{
            deviceInfo = Pa_GetDeviceInfo(i);
#ifdef __linux__
            const char* name = deviceInfo->name;
            if( deviceInfo->maxOutputChannels == 24 && strstr(name, substr) != NULL)
                motu = i;
#else
			hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
			std::string deviceName(deviceInfo->name);
			std::string deviceHostName(hostInfo->name);

			if (deviceInfo->maxOutputChannels == 24 && deviceName.compare(motuName) == 0)
			{
				motu = i;
			}

#endif
        }
        if(motu != paNoDevice){
            printf("Found MOTU with name %s and %d channels\n", Pa_GetDeviceInfo(motu)->name, Pa_GetDeviceInfo(motu)->maxOutputChannels);
        }
        return motu;
    }


    //Try to start a playback session
    TapsError MotuPlayer::startPlaybackSession()
    {
        PaStreamParameters outputParameters;
        PaError err;

        err = Pa_Initialize();
        if(err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorInitializing;
        }

		PaDeviceIndex motuIndex = getMotuIndex();
        outputParameters.device = motuIndex;
        if (outputParameters.device == paNoDevice) {
            Pa_Terminate();
            return TapsNoMotuFound;
        }

        outputParameters.channelCount = 24;       
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream(
                &stream,
                NULL, /* no input */
                &outputParameters,
                SAMPLE_RATE,
                FRAMES_PER_BUFFER,
                paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                paCallback,
                getInstance());
        
        if(err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorOpeningStream;
        }

        err = Pa_SetStreamFinishedCallback(stream, &streamFinishedCallback);
        if(err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorSettingFinishedStreamCallback;
        }

        err = Pa_StartStream(stream);
        if(err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorStartingStream;
        }

        return TapsNoError;

    }

    //Try to stop the playback session
    TapsError MotuPlayer::stopPlaybackSession()
    {
        PaError err;
        err = Pa_StopStream(stream);
        if (err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorStoppingStream;
        }

        err = Pa_CloseStream(stream);
        if(err != paNoError)
        {
            Pa_Terminate();
            return TapsErrorClosingStream;
        }

        Pa_Terminate();
		sessionStarted = false;
        return TapsNoError;
    }


    //Returns if the session started or not
    bool MotuPlayer::successfulStart()
    {
        return sessionStarted;
    }

    //Signal a callback that determines the end of a symbol played
    void MotuPlayer::signalSymbolCallback(TapsError err)
    {
        playing = false;
#ifdef __linux__
        pthread_mutex_unlock(&motu_lock);
#else
		motu_lock = false;
#endif
        if(symbolCallback != 0)
        {
            symbolCallback(err); 
        }
        
    }

    //Signal a callback that determines the end of a sentence played
    void MotuPlayer::signalSentencePlayedCallback(TapsError err)
    {
        if(sequenceCallback != 0)
        {
            sequenceCallback(err);
        }
        else
        {
            printf("No callback registered for a sequence, played sequence with code %d\n", err);
        }
        
    }

    //Register an external callback to trigger when a symbol is played
    void MotuPlayer::registerSymbolPlayedCallback(SymbolPlayedCallback callback)
    {
        symbolCallback = callback;
    }

    //Register an external callback to trigger when a sequence is played
    void MotuPlayer::registerSequencePlayedCallback(SequencePlayedCallback callback)
    {
        sequenceCallback = callback;
    }

    //Get the registered callback for playing a symbol
    SymbolPlayedCallback MotuPlayer::getRegisteredSymbolCallback()
    {
        return symbolCallback;
    }

    //Is currently playing
    bool MotuPlayer::isPlaying()
    {
        return playing;
    }

	//Stop the stream
	void MotuPlayer::interruptStream()
	{
		PaError err = Pa_StopStream(stream);
		if (err != paNoError)
		{
			printf("Could not interrupt the stream\n");
		}
	}

	//Is the stream active
	bool MotuPlayer::isStreamActive()
	{
		return Pa_IsStreamActive(stream);
	}

	//Start the stream
	void MotuPlayer::startStream()
	{
		PaError err = Pa_StartStream(stream);
		if (err != paNoError)
		{
			printf("Could not start the stream\n");
		}
	}

	//Set the parameters for the periods of silence
	void MotuPlayer::setSilenceParameters(int ici, int iwi)
	{
		if (ici != lastICI)
		{
			iciSilence->resetAsSilence(ici, SAMPLE_RATE);
			lastICI = ici;
		}
		
		if (iwi != lastIWI)
		{
			iwiSilence->resetAsSilence(iwi, SAMPLE_RATE);
			lastIWI = iwi;
		}
	}

	//Play ICI period
	void MotuPlayer::playICI()
	{
		iciSilence->resetIndex();
		currentPlayingSymbol = iciSilence;
	}

	//Play IWI period
	void MotuPlayer::playIWI()
	{
		iwiSilence->resetIndex();
		currentPlayingSymbol = iwiSilence;
	}

    //Trim a string
#ifdef __linux__
    std::string trim(std::string str)
    {
        // remove trailing white space
        while( !str.empty() && std::isspace( str.back() ) ) str.pop_back() ;

        // return residue after leading white space
        std::size_t pos = 0 ;
        while( pos < str.size() && std::isspace( str[pos] ) ) ++pos ;
        return str.substr(pos) ;
    }
#endif

    //Playback functionality/////////////////////////////////////////

    //Play a haptic symbol with a string code
    //WARNING: This method is not synchronized, playHapticSymbol is safer
    TapsError MotuPlayer::playSymbol(std::string code)
    {   
        if(!sessionStarted)
        {
            signalSymbolCallback(TapsErrorNoPlaybackSession);
        }
        else if(!playing)
        {
            std::unordered_map<std::string, HapticSymbol*>::iterator it;
			
            //If its a phoneme
            it = phonemes->find(code);
            if(it != phonemes->end())
            {
                HapticSymbol* phoneme = it->second;
                phoneme->resetIndex();
                playing = true;
                currentPlayingSymbol = phoneme;
                return TapsNoError;
            }

            //If its a flag
            it = flags->find(code);
            if(it != flags->end())
            {
                HapticSymbol* flag = it->second;
                flag->resetIndex();
                playing = true;
                currentPlayingSymbol = flag;
                return TapsNoError;
            }

            //If its a chunk
            it = chunks->find(code);
            if(it != chunks->end())
            {
                HapticSymbol* chunk = it->second;
                chunk->resetIndex();
                playing = true;
                currentPlayingSymbol = chunk;
                return TapsNoError;
            }
			signalSymbolCallback(TapsErrorSymbolNotFound);
            return TapsErrorSymbolNotFound;
        }
        else
        {
            signalSymbolCallback(TapsErrorSignalAlreadyPlaying);
            return TapsErrorSignalAlreadyPlaying;
        }
          
    }

    //Play a haptic symbol trying to lock MOTU
    TapsError MotuPlayer::playHapticSymbol(std::string code)
    {
#ifdef __linux__
        if(pthread_mutex_trylock(&motu_lock) == 0)
        {
            return playSymbol(code);
        }
        else 
            return TapsErrorSignalAlreadyPlaying;
#else
		if (!motu_lock)
		{
			motu_lock = true;
			return playSymbol(code);
		}
        else
        {
            return TapsErrorSignalAlreadyPlaying;
        }
        
#endif
    }

    //Sync callback to play a sequence of symbols
    //NOTE: this is not a method of the player class
    void syncCallback(TapsError err)
    {
        sequenceStruct->err = err;
		if (sequenceStruct->startFlagCallback != 0)
		{
			sequenceStruct->startFlagCallback(err);
			sequenceStruct->startFlagCallback = 0;
		}
#ifdef __linux__
        pthread_mutex_lock(&sequenceStruct.lock);
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&sequenceStruct.lock);
#else
		EnterCriticalSection(&lock);
		WakeConditionVariable(&condition);
		LeaveCriticalSection(&lock);
#endif    
    }

    //Thread process to play a sequence of symbols in linux
    //NOTE: this is not a method of the player class
#ifdef __linux__
    void* playSequenceProcessLinux(void* data)
    {
        MotuPlayer* player = MotuPlayer::getInstance();
        previousSymbolCallback = player->getRegisteredSymbolCallback();
        player->registerSymbolPlayedCallback(syncCallback);
        std::vector<std::string>::iterator it = sequenceStruct.sequence.begin();
		TapsError err;
		player->setSilenceParameters(sequenceStruct.ici, sequenceStruct.iwi);
        while (it != sequenceStruct.sequence.end() && sequenceStruct.err == TapsNoError)
        {
            std::string symbol = trim(*it);
            if(std::string(symbol).compare("PAUSE") == 0)
            {
				if (sequenceStruct.iwi != 0)
				{
					pthread_mutex_lock(&lock);
					player->playIWI();
					pthread_cond_wait(&condition, &lock);
					pthread_mutex_unlock(&lock);
				}
            }
            else
            {
				pthread_mutex_lock(&lock);
				err = player->playHapticSymbol(symbol);
				pthread_cond_wait(&condition, &lock);
				pthread_mutex_unlock(&lock);

                if(err == TapsNoError)
                {
					if (it + 1 != sequenceStruct.sequence.end())
					{
						if (sequenceStruct.ici != 0)
						{
							pthread_mutex_lock(&lock);
							player->playICI();
							pthread_cond_wait(&condition, &lock);
							pthread_mutex_unlock(&lock);
						}
					}
                }
                
            }
            it++;
        }
        player->signalSentencePlayedCallback(sequenceStruct.err);
        player->registerSymbolPlayedCallback(previousSymbolCallback);
        pthread_exit(NULL);
    }
#endif

	//Thread process to play a sequence of symbols in Windows
	//NOTE: this is not a method of the player class
#ifdef _WIN32
	DWORD playSequenceProcessWin(LPVOID lpParam)
	{
		DWORD waitResult = WaitForSingleObject(sequenceMutex, 0);
		if (waitResult == WAIT_OBJECT_0)
		{
			sequenceStruct = (SequenceStructure*)lpParam;
			MotuPlayer* player = MotuPlayer::getInstance();
			previousSymbolCallback = player->getRegisteredSymbolCallback();
			player->registerSymbolPlayedCallback(syncCallback);
			std::vector<std::string>::iterator it = sequenceStruct->sequence.begin();
			TapsError err;
			player->setSilenceParameters(sequenceStruct->ici, sequenceStruct->iwi);
			while (it != sequenceStruct->sequence.end() && sequenceStruct->err == TapsNoError)
			{
				std::string symbol = *it;
				if (std::string(symbol).compare("PAUSE") == 0)
				{
					if (sequenceStruct->iwi != 0)
					{
						EnterCriticalSection(&lock);
						player->playIWI();
						SleepConditionVariableCS(&condition, &lock, INFINITE);
						LeaveCriticalSection(&lock);
					}

				}
				else
				{
					EnterCriticalSection(&lock);
					err = player->playHapticSymbol(symbol);
					SleepConditionVariableCS(&condition, &lock, INFINITE);
					LeaveCriticalSection(&lock);
					if (err == TapsNoError)
					{

						if (it + 1 != sequenceStruct->sequence.end())
						{
							if (sequenceStruct->ici != 0)
							{
								//At this point, we are not sure if all data has been played on the tactors
								EnterCriticalSection(&lock);
								player->playICI();
								SleepConditionVariableCS(&condition, &lock, INFINITE);
								LeaveCriticalSection(&lock);
							}
						}
					}
				}
				it++;
			}
			player->signalSentencePlayedCallback(sequenceStruct->err);
			player->registerSymbolPlayedCallback(previousSymbolCallback);
			delete sequenceStruct;
			sequenceStruct = NULL;
			ReleaseMutex(sequenceMutex);
			return 0;
		}
		else
		{
			return -1;
		}
	}
#endif


    //Play a sequence of symbols including possible pauses for words
    void MotuPlayer::playSequence(std::vector<std::string> sequence, int ici, int iwi, StartFlagPlayedCallback startFlagCallback, std::string startFlag)
    {
		SequenceStructure *newSequenceStruct = new SequenceStructure;
		newSequenceStruct->sequence = sequence;
		newSequenceStruct->ici = ici;
		newSequenceStruct->iwi = iwi;
		newSequenceStruct->err = TapsNoError;
		newSequenceStruct->startFlagCallback = startFlagCallback;
		newSequenceStruct->startFlag = startFlag;
		if (startFlagCallback != 0)
			newSequenceStruct->sequence.insert(newSequenceStruct->sequence.begin(), startFlag);

		newSequenceStruct->sentence = "";
		newSequenceStruct->sentenceTranscribedCallback = 0;
#ifdef __linux__
        pthread_t thread;
        if(pthread_create(&thread, NULL, playSequenceProcessLinux, NULL) == 0)
        {
            pthread_detach(thread);
        }
        else
        {
            printf("Could not create the thread process\n");
        }
#else

		CreateThread(NULL, 0, playSequenceProcessWin, newSequenceStruct, 0, NULL);
		
#endif
        
    }

#ifdef _WIN32

	DWORD WINAPI playSentenceProcessWin(LPVOID lpParam)
	{
		DWORD waitResult = WaitForSingleObject(sequenceMutex, 0);
		if (waitResult == WAIT_OBJECT_0)
		{
			sequenceStruct = (SequenceStructure*)lpParam;
			std::vector<std::string> phonemes;
			MotuPlayer* player = MotuPlayer::getInstance();
			player->getPhonemesOfSentence(&phonemes, sequenceStruct->sentence);
			if (sequenceStruct->sentenceTranscribedCallback != 0)
				sequenceStruct->sentenceTranscribedCallback();
			sequenceStruct->sequence = phonemes;
			sequenceStruct->err = TapsNoError;
			player->setSilenceParameters(sequenceStruct->ici, sequenceStruct->iwi);
			if (sequenceStruct->startFlagCallback != 0)
				sequenceStruct->sequence.insert(sequenceStruct->sequence.begin(), sequenceStruct->startFlag);

			previousSymbolCallback = player->getRegisteredSymbolCallback();
			player->registerSymbolPlayedCallback(syncCallback);
			std::vector<std::string>::iterator it = sequenceStruct->sequence.begin();

			while (it != sequenceStruct->sequence.end() && sequenceStruct->err == TapsNoError)
			{
				std::string symbol = *it;
				if (std::string(symbol).compare("PAUSE") == 0)
				{
					if (sequenceStruct->iwi != 0)
					{
						EnterCriticalSection(&lock);
						player->playIWI();
						SleepConditionVariableCS(&condition, &lock, INFINITE);
						LeaveCriticalSection(&lock);
					}
				}
				else
				{
					EnterCriticalSection(&lock);
					player->playHapticSymbol(symbol);
					SleepConditionVariableCS(&condition, &lock, INFINITE);
					LeaveCriticalSection(&lock);
					if (it + 1 != sequenceStruct->sequence.end())
					{
						if (sequenceStruct->ici != 0)
						{
							//At this point, we are not sure if all data has been played on the tactors
							EnterCriticalSection(&lock);
							player->playICI();
							SleepConditionVariableCS(&condition, &lock, INFINITE);
							LeaveCriticalSection(&lock);
						}
					}
				}
				it++;
			}
			player->signalSentencePlayedCallback(sequenceStruct->err);
			player->registerSymbolPlayedCallback(previousSymbolCallback);
			delete sequenceStruct;
			sequenceStruct = NULL;
			ReleaseMutex(sequenceMutex);
			return 0;
		}
		else
		{
			return -1;
		}

	}
#endif

    //Play a sequence of symbols with no word separation
    void MotuPlayer::playSymbolSequence(std::vector<std::string> sequence, int ici)
    {
        playSequence(sequence, ici, 0, 0, "");
    }

	//Play a sequence of symbols with no word separation
	void MotuPlayer::playSymbolSequence(std::vector<std::string> sequence, int ici, StartFlagPlayedCallback startFlagCallback, std::string startFlag)
	{
		playSequence(sequence, ici, 0, startFlagCallback, startFlag);
	}

    //Play a sentence written in English using Flite
    void MotuPlayer::playEnglishSentence(std::string sentence, int ici, int iwi)
    {
		playEnglishSentence(sentence, ici, iwi, 0, "", 0);
    }

	//Play a sentence written in English using Flite
	void MotuPlayer::playEnglishSentence(std::string sentence, int ici, int iwi, StartFlagPlayedCallback startFlagCallback, std::string startFlag, SentenceTranscribedCallback sentenceTranscribedCallback)
	{
#ifdef _WIN32
		SequenceStructure *newSequenceStruct = new SequenceStructure;
		newSequenceStruct->ici = ici;
		newSequenceStruct->iwi = iwi;
		newSequenceStruct->sentence = sentence;
		newSequenceStruct->startFlagCallback = startFlagCallback;
		newSequenceStruct->startFlag = startFlag;
		newSequenceStruct->sentenceTranscribedCallback = sentenceTranscribedCallback;
		CreateThread(NULL, 0, playSentenceProcessWin, newSequenceStruct, 0, NULL);
#else
		std::vector<std::string> phonemes;
		getPhonemesOfSentence(&phonemes, sentence);
		if (sentenceTranscribedCallback != 0)
			sentenceTranscribedCallback();
		playSequence(phonemes, ici, iwi, startFlagCallback, startFlag);
#endif
	}

    // TTS functionality ///////////////////////////////////////////////////

    //Get the raw phoneme transcription of flite as a string 
    std::string MotuPlayer::getRawFlitePhonemes(std::string sentence)
    {
		std::string result;
		std::string command = "flite -t \"" + sentence + "\" -ps -o none";
#ifdef __linux__
        std::array<char, 1024> buffer;
    
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        //Remove last newline character
        result.erase(result.length()-1);

        //Remove first and last "pau" symbols and the corresponding spaces
        int i;
        for(i = 0; i < 4; i++)
            result.erase(result.begin());
        for(i = 0; i < 5; i++)
            result.erase(result.length()-1);

        return result;
#else

		HANDLE g_hChildStd_OUT_Rd = NULL;
		HANDLE g_hChildStd_OUT_Wr = NULL;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		{
			printf("Could not create pipe\n");
			return "";
		}

		if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		{
			printf("Read handle inherited\n");
			return "";
		}

		//Child process
		STARTUPINFO StartupInfo;
		PROCESS_INFORMATION ProcessInfo;
		char Args[4096];
		char *pEnvCMD = NULL;
		const char *pDefaultCMD = "CMD.EXE";
		ULONG rc;

		memset(&StartupInfo, 0, sizeof(StartupInfo));
		StartupInfo.cb = sizeof(STARTUPINFO);
		StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
		StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
		StartupInfo.wShowWindow = SW_HIDE;
		StartupInfo.hStdOutput = g_hChildStd_OUT_Wr;

		Args[0] = 0;
		strcpy_s(Args, pDefaultCMD);

		// "/c" option - Do the command then terminate the command window
		strcat_s(Args, " /c ");
		//the flite application and the arguments
		strcat_s(Args, command.c_str());


		BOOL bSuccess = FALSE;
		bSuccess = CreateProcess(NULL,
			Args,     // command line 
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			0,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&StartupInfo,  // STARTUPINFO pointer 
			&ProcessInfo);  // receives PROCESS_INFORMATION 

		if (!bSuccess)
		{
			printf("Could not create process\n");
			return "";
		}
		else
		{

			WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
			if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
				rc = 0;

			CloseHandle(ProcessInfo.hThread);
			CloseHandle(ProcessInfo.hProcess);

		}

		//Read the pipe
		DWORD dwRead;
		CHAR chBuf[4096];
		bSuccess = FALSE;
		HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		int i;
		for (;;)
		{
			bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
			i = 0;
			while (i < dwRead)
			{
				result += chBuf[i];
				i++;
			}
			if (!bSuccess || dwRead < 4096) break;
		}

		//Remove last newline character
		result.erase(result.length() - 1);

		//Remove first and last "pau" symbols and the corresponding spaces
		for (i = 0; i < 4; i++)
			result.erase(result.begin());
		for (i = 0; i < 5; i++)
			result.erase(result.length() - 1);

		return result;

#endif

    }

    //Split a string by a delimiter and store the results in the given vector
    void MotuPlayer::splitStringBy(std::vector<std::string>* result, std::string delimiter, std::string str)
    {
        size_t pos = 0;
        std::string token;
        while((pos = str.find(delimiter)) != std::string::npos)
        {
            token = str.substr(0,pos);
            result->push_back(token);
            str.erase(0, pos + delimiter.length());
        }
        result->push_back(str);
    }

    //Get phonemes from a sentence
    void MotuPlayer::getPhonemesOfSentence(std::vector<std::string>* result, std::string sentence)
    {
        //Split the sentence into words
        std::vector<std::string> words;
        splitStringBy(&words, " ", sentence);

        //For every word, get the phoneme transcription
        std::vector<std::string> phonemes;
        std::string flitePhonemes;
        std::vector<std::string>::iterator itPhonemes;
        std::vector<std::string>::iterator it = words.begin();
        std::string word;
        std::string flitePhoneme;
        std::string mappedSymbol;
        std::string firstPhoneme;
        std::string secondPhoneme;
        while(it != words.end())
        {
            word = *it;
            //Flite phonemes in a vector
            flitePhonemes = getRawFlitePhonemes(word);
            phonemes.clear();
            splitStringBy(&phonemes, " ", flitePhonemes);

            //For every flite phoneme, get the mapped symbol in our codes
            itPhonemes = phonemes.begin();
            size_t posPhoneme;
            while(itPhonemes != phonemes.end())
            {
                flitePhoneme = *itPhonemes;
                mappedSymbol = (fliteMapping.find(flitePhoneme))->second;

                posPhoneme = mappedSymbol.find("-");
                if(posPhoneme != std::string::npos)
                {
                    firstPhoneme = mappedSymbol.substr(0, posPhoneme);
                    secondPhoneme = mappedSymbol.substr(posPhoneme+1, mappedSymbol.size());
                    result->push_back(firstPhoneme);
                    result->push_back(secondPhoneme);
                }
                else
                {
                    result->push_back(mappedSymbol);
                }
                ++itPhonemes;
            }
            if((it+1) != words.end())
                result->push_back("PAUSE");
            it++;
        }

    }

    

}