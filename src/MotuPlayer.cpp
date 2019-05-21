#include "../inc/MotuPlayer.hpp"

namespace TapX
{

    //Mutex
#ifdef linux
    pthread_mutex_t motu_lock;
#else
	std::mutex motu_lock;
#endif

    //Structure of a sequence of symbols
    SequenceStructure sequenceStruct;

    //Previously set callback for playing symbols
    SymbolPlayedCallback previousSymbolCallback = 0;

    //Is motu playing
#ifdef linux
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
#else
	std::condition_variable condition;
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
#ifdef linux
        pthread_mutex_destroy(&motu_lock);
#endif
        stopPlaybackSession();
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
        initializeData();
            
    };

    //Start the playback session
    void MotuPlayer::startSession()
    {
        //Try to start the playback session
        TapsError err = startPlaybackSession();
        sessionStarted = err == TapsNoError;
        int attempts = 0;

        while (!sessionStarted && attempts < 3)
        {
            printf("ERROR: Attempt %d, could not start playback session with code %d\n", attempts, err);
            err = startPlaybackSession();
            sessionStarted = err == TapsNoError;
            attempts ++;
        }

        if(sessionStarted)
            printf("Playback session started successfuly\n");
        else
        {
            printf("ERROR: Could not start playback session after 3 attempts\n");
        }
    }

    //Get files on a given path
    int MotuPlayer::intializeMap(std::string path, std::unordered_map<std::string, HapticSymbol*> &map)
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
			copied = GetModuleFileNameW(0, &pathBuf.at(0), pathBuf.size());
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
				symbol->initializeData(basePath + symbolName);
				std::pair<std::string, HapticSymbol*> newPair(symbolName, symbol);
				map.insert(newPair);
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
        file.open(FLITE_MAP_PATH);
        std::string delimiter = ",";
        std::string first;
        std::string second;
        size_t pos = 0;
        while(file.good())
        {
            getline(file,line);
            pos = line.find(delimiter);
            first = line.substr(0, pos);
            second = line.substr(pos+1, line.size());
            
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
        unsigned long i;
        (void) timeInfo; /* Prevent unused variable warnings. */
        (void) statusFlags;
        (void) inputBuffer;
        float v;
        int k;
   
        for( i=0; i<framesPerBuffer; i++ )
        {
            for(k = 0; k < 24; k++)
            {
                if(symbol != 0 && !symbol->matrixConsumed())
                {
                    v = symbol->getValueAt(symbol->getMatrixRowIndex(), k);
                    *out++ = v;
                }
                else
                    *out++ = 0.0;
            }
            if(symbol != 0 && ! symbol->increaseIndex())
            {
                player->signalSymbolCallback(TapsNoError);
                player->currentPlayingSymbol = 0;
                break;
            }
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
        for(int i = 0; i < numDevices; i++){
            deviceInfo = Pa_GetDeviceInfo(i);
            const char* name = deviceInfo->name;
            const char* substr = "24Ao";

            if( deviceInfo->maxOutputChannels == 24 && strstr(name, substr) != NULL)
                motu = i;
        }
        if(motu != paNoDevice){
            printf("Found MOTU with name %s and channels %d\n", Pa_GetDeviceInfo(motu)->name, Pa_GetDeviceInfo(motu)->maxOutputChannels);
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

        outputParameters.device = getMotuIndex();
        if (outputParameters.device == paNoDevice) {
            Pa_Terminate();
            return TapsNoMotuFound;
        }

        outputParameters.channelCount = 24;       
        outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
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
#ifdef linux
        pthread_mutex_unlock(&motu_lock);
#else
		motu_lock.unlock();
#endif
        if(symbolCallback != 0)
        {
            symbolCallback(err); 
        }
        else
        {
            printf("No callback registered, played symbol with code %d\n", err);
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

    //Playback functionality/////////////////////////////////////////

    //Play a haptic symbol with a string code
    //WARNING: This method is not synchronized, playHapticSymbol is safer
    void MotuPlayer::playSymbol(std::string code)
    {   
        if(!sessionStarted)
        {
            signalSymbolCallback(TapsErrorNoPlaybackSession);
        }
        else if(!playing)
        {
            std::unordered_map<std::string, HapticSymbol*>::iterator it;

            //If its a phoneme
            it = phonemes.find(code);
            if(it != phonemes.end())
            {
                HapticSymbol* phoneme = it->second;
                phoneme->resetIndex();
                playing = true;
                currentPlayingSymbol = phoneme;
                return;
            }

            //If its a flag
            it = flags.find(code);
            if(it != flags.end())
            {
                HapticSymbol* flag = it->second;
                flag->resetIndex();
                playing = true;
                currentPlayingSymbol = flag;
                return;
            }

            //If its a chunk
            it = chunks.find(code);
            if(it != chunks.end())
            {
                HapticSymbol* chunk = it->second;
                chunk->resetIndex();
                playing = true;
                currentPlayingSymbol = chunk;
                return;
            }
            
            signalSymbolCallback(TapsErrorSymbolNotFound);
        }
        else
        {
            signalSymbolCallback(TapsErrorSignalAlreadyPlaying);
        }
          
    }

    //Play a haptic symbol trying to lock MOTU
    void MotuPlayer::playHapticSymbol(std::string code)
    {
#ifdef linux
        if(pthread_mutex_trylock(&motu_lock) == 0)
        {
            playSymbol(code);
        }
#else
		if (motu_lock.try_lock())
		{
			playSymbol(code);
		}
#endif
    }

    //Sync callback to play a sequence of symbols
    //NOTE: this is not a method of the player class
    void syncCallback(TapsError err)
    {
        sequenceStruct.err = err;
#ifdef linux
        pthread_mutex_lock(&sequenceStruct.lock);
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&sequenceStruct.lock);
#else
		std::unique_lock<std::mutex> lock(sequenceStruct.lock);
		condition.notify_one();
#endif
    }

    //Thread process to play a sequence of symbols
    //NOTE: this is not a method of the player class
    void* playSequenceProcess(void* data)
    {
        MotuPlayer* player = MotuPlayer::getInstance();
        previousSymbolCallback = player->getRegisteredSymbolCallback();
        player->registerSymbolPlayedCallback(syncCallback);
        std::vector<std::string>::iterator it = sequenceStruct.sequence.begin();

        while (it != sequenceStruct.sequence.end() && sequenceStruct.err == TapsNoError)
        {
            std::string symbol = *it;
            if(std::string(symbol).compare("PAUSE") == 0)
            {
#ifdef linux
                usleep(sequenceStruct.iwi*1000);
#else
				std::this_thread::sleep_for(std::chrono::milliseconds(sequenceStruct.iwi));
#endif
            }
            else
            {
#ifdef linux
                pthread_mutex_lock(&sequenceStruct.lock);
                player->playHapticSymbol(symbol);
                pthread_cond_wait(&condition, &sequenceStruct.lock);
                if(it + 1 != sequenceStruct.sequence.end())
                    usleep(sequenceStruct.ici*1000);
                pthread_mutex_unlock(&sequenceStruct.lock);
#else
				std::unique_lock<std::mutex> lock(sequenceStruct.lock);
				player->playHapticSymbol(symbol);
				condition.wait(lock);
				if (it + 1 != sequenceStruct.sequence.end())
					std::this_thread::sleep_for(std::chrono::milliseconds(sequenceStruct.ici));
#endif
            }
            it++;
        }
        player->signalSentencePlayedCallback(sequenceStruct.err);
        player->registerSymbolPlayedCallback(previousSymbolCallback);
#ifdef linux
        pthread_exit(NULL);
#else
		std::terminate();
#endif
    }

    //Play a sequence of symbols including possible pauses for words
    void MotuPlayer::playSequence(std::vector<std::string> sequence, int ici, int iwi)
    {
        sequenceStruct.sequence = sequence;
        sequenceStruct.ici = ici;
        sequenceStruct.iwi = iwi;
        sequenceStruct.err = TapsNoError;
#ifdef linux
        pthread_t thread;
        if(pthread_create(&thread, NULL, playSequenceProcess, NULL) == 0)
        {
            pthread_detach(thread);
        }
        else
        {
            printf("Could not create the thread process\n");
        }
#else
		std::thread thread(playSequenceProcess, NULL);
		thread.detach();
#endif
        
    }

    //Play a sequence of symbols with no word separation
    void MotuPlayer::playSymbolSequence(std::vector<std::string> sequence, int ici)
    {
        playSequence(sequence, ici, 0);
    }

    //Play a sentence written in English using Flite
    void MotuPlayer::playEnglishSentence(std::string sentence, int ici, int iwi)
    {
        std::vector<std::string> phonemes;
        getPhonemesOfSentence(&phonemes, sentence);
        playSequence(phonemes, ici, iwi);
    }

    // TTS functionality ///////////////////////////////////////////////////

    //Get the raw phoneme transcription of flite as a string 
    std::string MotuPlayer::getRawFlitePhonemes(std::string sentence)
    {
#ifdef linux
        std::array<char, 128> buffer;
        std::string result;
        std::string command = "flite -t \"" + sentence + "\" -ps -o none";
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
		//TODO: Windows implementation
		return "P";
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