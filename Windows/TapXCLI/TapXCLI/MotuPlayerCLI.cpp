#include "MotuPlayerCLI.h"

using namespace System::Runtime::InteropServices;
using namespace TapX;

namespace TapXCLI
{
	//Memory handles for managed callbacks
	GCHandle gchSymbol;
	GCHandle gchSequence;

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
		gchSymbol.Free();
		
	}

	//Finalizer
	MotuPlayerCLI::!MotuPlayerCLI()
	{
		if (playerInstance != nullptr)
			delete playerInstance;
		gchSymbol.Free();
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

	//Play a sequence of symbols with a defined ICI
	void MotuPlayerCLI::PlaySequenceOfSymbols(array<String^>^ sequence, int ICI)
	{
		std::vector<std::string> vect(sequence->Length);
		if (sequence->Length)
		{
			msclr::interop::marshal_context context;
			cli::pin_ptr<String^> pinned = &sequence[0];
			for (int i = 0; i < sequence->Length; ++i) {
				vect[static_cast<size_t>(i)] = context.marshal_as<std::string>(sequence[i]);
			}
			playerInstance->playSymbolSequence(vect, ICI);
		}
	}

	//Play an English sentence using Flite with a defined ICI and IWI
	void MotuPlayerCLI::PlayEnglishSentence(String^ sentence, int ICI, int IWI)
	{
		std::string str_sentence((const char*)Marshal::StringToHGlobalAnsi(sentence).ToPointer());
		playerInstance->playEnglishSentence(str_sentence, ICI, IWI);
	}




}