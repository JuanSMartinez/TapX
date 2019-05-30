#include "MotuPlayerCLI.h"

using namespace System::Runtime::InteropServices;
using namespace TapX;

namespace TapXCLI
{
	GCHandle gch;
	void MotuPlayerCLI::InternalCallback(TapX::TapsError err)
	{
		printf("Hey!! %d\n", err);
		
		external->Invoke((int)err);
	}

	//Constructor
	MotuPlayerCLI::MotuPlayerCLI()
	{
		playerInstance = MotuPlayer::getInstance();
		playerInstance->startSession();

		if (SessionStarted())
		{
			SymbolPlayedCallbackCLI^ fp = gcnew SymbolPlayedCallbackCLI(this, &MotuPlayerCLI::InternalCallback);
			gch = GCHandle::Alloc(fp);
			IntPtr ip = Marshal::GetFunctionPointerForDelegate(fp);

			TapX::SymbolPlayedCallback cb = static_cast<TapX::SymbolPlayedCallback>(ip.ToPointer());
			playerInstance->registerSymbolPlayedCallback(cb);
			
		}
		
	}

	//Destructor
	MotuPlayerCLI::~MotuPlayerCLI()
	{
		if (playerInstance != nullptr)
			delete playerInstance;
		gch.Free();
		
	}

	//Finalizer
	MotuPlayerCLI::!MotuPlayerCLI()
	{
		if (playerInstance != nullptr)
			delete playerInstance;
		gch.Free();
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

	void MotuPlayerCLI::RegisterExternalCallback(SymbolCallback^ callback)
	{
		external = callback;
		
	}




}