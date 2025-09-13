#pragma once
#include <xaudio2.h>
#include <Windows.h> // TODO: do Linux version too
#include <wrl.h>

#include "Tout.hpp"

#define dp_NAME "Daft Punk          "
#define dp_NAME_ns "Daft_Punk"

#define dp_TRACE(...)	TRACE	(dp_NAME, __VA_ARGS__)
#define dp_INFO(...)	INFO	(dp_NAME, __VA_ARGS__)
#define dp_WARN(...)	WARN	(dp_NAME, __VA_ARGS__)
#define dp_ERROR(...)	OhSHIT	(dp_NAME, __VA_ARGS__)


// Merci https://learn.microsoft.com/en-us/windows/win32/xaudio2/full-project
// Constant literals. TODO: make this not constant
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



struct DaftPunk
{

	struct Engine
	{
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
	};

	static unsigned long PlaySoundFromFile(std::filesystem::path);
};