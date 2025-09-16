#pragma once
#include <xaudio2.h>
#include <Windows.h> // TODO: do Linux version too
#include <wrl.h>

#include "Tout.hpp"

#include <fmod.h>
#include <fmod_studio.hpp>
#include <fmod_errors.h>

#define dp_NAME "Daft Punk          "
#define dp_NAME_ns "Daft_Punk"

#define dp_TRACE(...)	TRACE	(dp_NAME, __VA_ARGS__)
#define dp_INFO(...)	INFO	(dp_NAME, __VA_ARGS__)
#define dp_WARN(...)	WARN	(dp_NAME, __VA_ARGS__)
#define dp_ERROR(...)	OhSHIT	(dp_NAME, __VA_ARGS__)

#define AUDIO_MAX_CHANNELS 512
#define AUDIO_TICK_RATE 16 // Audio will "tick" once every 16 milliseconds



// Merci https://learn.microsoft.com/en-us/windows/win32/xaudio2/full-project
// Constant literals. TODO: make this not constant
/*
constexpr WORD   BITSPERSSAMPLE = 16;                                                    // 16 bits per sample.
constexpr DWORD  SAMPLESPERSEC = 44100;                                                  // 44,100 samples per second.
constexpr double CYCLESPERSEC = 220.0;                                                   // 220 cycles per second (frequency of the audible tone).
constexpr double VOLUME = 0.5;                                                           // 50% volume.
constexpr WORD   AUDIOBUFFERSIZEINCYCLES = 10;                                           // 10 cycles per audio buffer.
constexpr double PI = 3.14159265358979323846;

// Calculated constants.
constexpr DWORD  SAMPLESPERCYCLE = (DWORD)(SAMPLESPERSEC / CYCLESPERSEC);                // 200 samples per cycle.
constexpr DWORD  AUDIOBUFFERSIZEINSAMPLES = SAMPLESPERCYCLE * AUDIOBUFFERSIZEINCYCLES;   // 2,000 samples per buffer.
constexpr UINT32 AUDIOBUFFERSIZEINBYTES = AUDIOBUFFERSIZEINSAMPLES * BITSPERSSAMPLE / 8; // 4,000 bytes per buffer.
*/



struct DaftPunk
{

	struct Engine // TODO : Delete this and use Player instead (FMOD)
	{
		/*
		class SoundCallbackHander : public IXAudio2VoiceCallback
		{
		public:
			// Voice callbacks from IXAudio2VoiceCallback
			STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 bytesRequired);
			STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS);
			STDMETHOD_(void, OnStreamEnd) (THIS);
			STDMETHOD_(void, OnBufferStart) (THIS_ void* bufferContext);
			STDMETHOD_(void, OnBufferEnd) (THIS_ void* bufferContext);
			STDMETHOD_(void, OnLoopEnd) (THIS_ void* bufferContext);
			STDMETHOD_(void, OnVoiceError) (THIS_ void* bufferContext, HRESULT error);

		private:
			inline static bool m_isPlayingHolder;
		};

		inline static SoundCallbackHander m_soundCallbackHandler;
		inline static bool b_IsInitialised = false;
		static bool Init(bool pForceInit = false);

		inline static Microsoft::WRL::ComPtr<IXAudio2> m_Instance;
		inline static IXAudio2MasteringVoice* m_MasteringVoice;
		inline static IXAudio2SourceVoice* m_SourceVoice;
		inline static XAUDIO2_BUFFER m_AudioBuffer;
		*/
	};

	struct Player
	{
		static bool Initialise();
		static void Shutdown();
		static UINT8 AllIsGood();
		static void Tick();

		inline static FMOD::Studio::System* System_Instance = nullptr;
		inline static std::atomic<bool> b_IsRunning = false;
		inline static std::atomic<bool> b_IsInitialised = false;
		inline static std::thread Tick_thread;

	private:
		/*
		inline static std::atomic<bool> s_keep_running{ false };
		inline static bool s_is_initialised = false;
		inline static FMOD::System* s_core_system = nullptr;
		inline static std::thread s_update_thread;
		inline static std::mutex s_initialise_mutex;
		inline static std::mutex s_bank_mutex;
		inline static std::mutex s_stream_mutex;
		inline static std::unordered_map<std::string, FMOD::Studio::Bank*> s_loaded_banks;
		inline static std::unordered_map<FMOD::Channel*, FMOD::Sound*> s_active_streams;
		*/
	};
};