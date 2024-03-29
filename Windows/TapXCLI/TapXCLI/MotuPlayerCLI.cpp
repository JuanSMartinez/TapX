#include "MotuPlayerCLI.h"

using namespace System::Runtime::InteropServices;
using namespace TapX;

namespace TapXCLI
{
	//Memory handles for managed callbacks
	GCHandle gchSymbol;
	GCHandle gchSequence;
	GCHandle gchStartFlag;
	GCHandle gchSentenceTranscribed;

	//Internal symbol callback. Serves as a bridge between the unmanaged and managed callbacks for symbols
	void MotuPlayerCLI::InternalSymbolCallback(TapX::TapsError err)
	{		
		if(externalSymbolCallback != nullptr)
			externalSymbolCallback->Invoke((int)err);
	}

	//Internal sequence callback. Serves as a bridge between the unmanaged and managed callbacks for sequences
	void MotuPlayerCLI::InternalSequenceCallback(TapX::TapsError err)
	{
		if (externalSequenceCallback != nullptr)
			externalSequenceCallback->Invoke((int)err);
	}

	//Internal callback for when a start flag is played
	void MotuPlayerCLI::InternalStartFlagCallback(TapX::TapsError err)
	{
		if (externalStartFlagCallback != nullptr)
		{
			externalStartFlagCallback->Invoke((int)err);
			externalStartFlagCallback = nullptr;
		}
	}

	//Constructor
	MotuPlayerCLI::MotuPlayerCLI()
	{
		playerInstance = MotuPlayer::getInstance();
		playerInstance->startSession();

		if (SessionStarted())
		{
			//Register internal callbacks

			//Symbols
			SymbolPlayedCallbackCLI^ fpSymbol = gcnew SymbolPlayedCallbackCLI(this, &MotuPlayerCLI::InternalSymbolCallback);
			gchSymbol = GCHandle::Alloc(fpSymbol);
			IntPtr ipSymbol = Marshal::GetFunctionPointerForDelegate(fpSymbol);

			TapX::SymbolPlayedCallback cbsSymbol = static_cast<TapX::SymbolPlayedCallback>(ipSymbol.ToPointer());
			playerInstance->registerSymbolPlayedCallback(cbsSymbol);

			//Sequences
			SequencePlayedCallbackCLI^ fpSequence = gcnew SequencePlayedCallbackCLI(this, &MotuPlayerCLI::InternalSequenceCallback);
			gchSequence = GCHandle::Alloc(fpSequence);
			IntPtr ipSequence = Marshal::GetFunctionPointerForDelegate(fpSequence);

			TapX::SequencePlayedCallback cbsSequence = static_cast<TapX::SequencePlayedCallback>(ipSequence.ToPointer());
			playerInstance->registerSequencePlayedCallback(cbsSequence);
			
		}
		
	}

	//Destructor
	MotuPlayerCLI::~MotuPlayerCLI()
	{
		if (playerInstance != nullptr)
			delete playerInstance;
		try
		{
			gchSymbol.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchSentenceTranscribed.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchSequence.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchStartFlag.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		
	}

	//Finalizer
	MotuPlayerCLI::!MotuPlayerCLI()
	{
		if (playerInstance != nullptr)
			delete playerInstance;
		try
		{
			gchSymbol.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchSentenceTranscribed.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchSequence.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		try
		{
			gchStartFlag.Free();
		}
		catch (Exception^ e)
		{
			//Thrown if the handle was not initialized, do nothing
		}
		
		
		
	}

	//Did the session start correctly
	bool MotuPlayerCLI::SessionStarted()
	{
		return playerInstance->successfulStart();
	}

	//Play a haptic symbol 
	void MotuPlayerCLI::PlayHapticSymbol(String^ code)
	{
		std::string str_code((const char*)Marshal::StringToHGlobalAnsi(code).ToPointer());
		playerInstance->playHapticSymbol(str_code);
	}

	//Register an external symbol played callback
	void MotuPlayerCLI::RegisterExternalSymbolCallback(SymbolCallback^ callback)
	{
		externalSymbolCallback = callback;
		
	}

	//Register an external sequence played callback
	void MotuPlayerCLI::RegisterExternalSequenceCallback(SequenceCallback^ callback)
	{
		externalSequenceCallback = callback;
	}

	//Play a sequence of symbols with a defined ICI with a start flag and start flag callback
	void MotuPlayerCLI::PlaySequenceOfSymbols(array<String^>^ sequence, int ICI, StartFlagCallback^ startFlagCallback, String^ startFlag)
	{
		std::vector<std::string> vect(sequence->Length);
		if (sequence->Length)
		{
			msclr::interop::marshal_context context;
			cli::pin_ptr<String^> pinned = &sequence[0];
			for (int i = 0; i < sequence->Length; ++i) {
				vect[static_cast<size_t>(i)] = context.marshal_as<std::string>(sequence[i]);
			}

			if (startFlagCallback != nullptr)
			{
				std::string str_startFlag((const char*)Marshal::StringToHGlobalAnsi(startFlag).ToPointer());
				StartFlagPlayedCallbackCLI^ fpSequence = gcnew StartFlagPlayedCallbackCLI(this, &MotuPlayerCLI::InternalStartFlagCallback);
				gchStartFlag = GCHandle::Alloc(fpSequence);
				IntPtr ipSequence = Marshal::GetFunctionPointerForDelegate(fpSequence);

				TapX::StartFlagPlayedCallback cbsSequence = static_cast<TapX::StartFlagPlayedCallback>(ipSequence.ToPointer());
				externalStartFlagCallback = startFlagCallback;
				playerInstance->playSymbolSequence(vect, ICI, cbsSequence, str_startFlag);
			}
			else
			{
				playerInstance->playSymbolSequence(vect, ICI);
			}
		}
	}

	//Play a sequence of symbols with a defined ICI
	void MotuPlayerCLI::PlaySequenceOfSymbols(array<String^>^ sequence, int ICI)
	{
		PlaySequenceOfSymbols(sequence, ICI, nullptr, "");
	}

	//Play an English sentence using Flite with a defined ICI, IWI, start flag and start flag callback
	void MotuPlayerCLI::PlayEnglishSentence(String^ sentence, int ICI, int IWI, StartFlagCallback^ startFlagCallback, String^ startFlag, SentenceTranscribedCallback^ sentenceTranscribedCallback)
	{
		std::string str_sentence((const char*)Marshal::StringToHGlobalAnsi(sentence).ToPointer());
		if (startFlagCallback != nullptr)
		{
			std::string str_startFlag((const char*)Marshal::StringToHGlobalAnsi(startFlag).ToPointer());
			StartFlagPlayedCallbackCLI^ fpSequence = gcnew StartFlagPlayedCallbackCLI(this, &MotuPlayerCLI::InternalStartFlagCallback);
			gchStartFlag = GCHandle::Alloc(fpSequence);
			IntPtr ipSequence = Marshal::GetFunctionPointerForDelegate(fpSequence);

			TapX::StartFlagPlayedCallback cbsSequence = static_cast<TapX::StartFlagPlayedCallback>(ipSequence.ToPointer());
			externalStartFlagCallback = startFlagCallback;

			if(sentenceTranscribedCallback == nullptr)
				playerInstance->playEnglishSentence(str_sentence, ICI, IWI, cbsSequence, str_startFlag, 0);
			else
			{
	
				gchSentenceTranscribed = GCHandle::Alloc(sentenceTranscribedCallback);
				IntPtr ipSequenceSentence = Marshal::GetFunctionPointerForDelegate(sentenceTranscribedCallback);
				TapX::SentenceTranscribedCallback cbsSequenceSentence = static_cast<TapX::SentenceTranscribedCallback>(ipSequenceSentence.ToPointer());
				playerInstance->playEnglishSentence(str_sentence, ICI, IWI, cbsSequence, str_startFlag, cbsSequenceSentence);
			}
		}
		else
		{
			playerInstance->playEnglishSentence(str_sentence, ICI, IWI);
		}
	}

	//Play an English sentence using Flite with a defined ICI and IWI
	void MotuPlayerCLI::PlayEnglishSentence(String^ sentence, int ICI, int IWI)
	{
		PlayEnglishSentence(sentence, ICI, IWI, nullptr, "", nullptr);
	}

	//Get raw flite phonemes of a sentence
	String^ MotuPlayerCLI::GetFlitePhonemesOf(String^ sentence)
	{
		std::string str_sentence((const char*)Marshal::StringToHGlobalAnsi(sentence).ToPointer());
		std::string flitePhonemes = playerInstance->getRawFlitePhonemes(str_sentence);
		return gcnew String(flitePhonemes.c_str());
	}

	//Get TAPS phonemes of a sentence
	array<String^>^ MotuPlayerCLI::GetPhonemesOf(String^ sentence)
	{
		std::string str_sentence((const char*)Marshal::StringToHGlobalAnsi(sentence).ToPointer());
		std::vector<std::string> result;
		playerInstance->getPhonemesOfSentence(&result, str_sentence);
		int i;
		array<String^>^ m_result = gcnew array<String^>(result.size());
		for (i = 0; i < result.size(); i++)
			m_result[i] = gcnew String(result.at(i).c_str());
		return m_result;

	}




}