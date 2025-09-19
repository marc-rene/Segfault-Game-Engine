#include "../include/DaftPunk.hpp"
#include <winerror.h>

// Thanks https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif




// ---------------------------------------------------------------------------------------
// Now we use FMOD
// ---------------------------------------------------------------------------------------

bool DaftPunk::Player::Initialise()
{
	b_IsInitialised.store(false);
	FMOD_RESULT result = FMOD::System_Create(&system_instance, FMOD_VERSION);

	switch (result)
	{
	case FMOD_OK:
		dp_INFO("Successfully made a FMOD system object");
		break;
	default:
		dp_ERROR("Could NOT make an FMOD system object because we got {} instead of FMOD_OK", FMOD_ErrorString(result));
		return false;
		break;
	}

	result = system_instance->init(AUDIO_MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
	switch (result)
	{
	case FMOD_OK:
		dp_INFO("Successfully Initialised our FMOD system");
		break;
	default:
		dp_ERROR("Could NOT Init our FMOD system because we got {} instead of FMOD_OK", FMOD_ErrorString(result));
		return false;
		break;
	}

	try
	{
		tick_thread = std::thread([]()
			{
				while (DaftPunk::Player::b_IsRunning.load())
				{
					DaftPunk::Player::Tick();
					std::this_thread::sleep_for(std::chrono::milliseconds(AUDIO_TICK_RATE));
				}
			});

		b_IsInitialised.store(true);
	}
	catch (const std::exception& e)
	{
		dp_ERROR("DaftPunk player thread messed up!!\nsaying: {}", e.what());
		b_IsRunning.store(false);
		b_IsInitialised.store(false);

		if (system_instance != nullptr)
		{
			system_instance->release();
			system_instance->close();
			system_instance = nullptr;
		}
		return false;
	}

	return b_IsInitialised.load();
}


void DaftPunk::Player::Shutdown()
{
	dp_INFO("Shutting down FMOD Studio player");

	b_IsRunning.store(false);
	b_IsInitialised.store(false);
	if (tick_thread.joinable())
		tick_thread.join();

	tick_thread.~thread();

	//UnloadAllBanks();

	if (system_instance != nullptr)
	{
		system_instance->release();
		system_instance->close();
	}
}


UINT8 DaftPunk::Player::AllIsGood()
{
	if (system_instance == nullptr)
	{
		dp_ERROR("Our System instance is banjaxxed and null");
		return 1;
	}
	if (b_IsInitialised.load() == false)
	{
		dp_ERROR("We have never been initialised, please run DaftPunk::Player::Initialise() and pray it returns true");
		return 2;
	}


	return 0; // All Quiet on the Daft Front
}

void DaftPunk::Player::Tick()
{
	if (AllIsGood() != 0)
	{
		dp_WARN("God DAMN we got SOMETHING happening!");
		return;
	}

	system_instance->update();

	std::scoped_lock stream_lock(mutex_stream);
	for (const std::pair<FMOD::Channel*, FMOD::Sound*>& stream : active_streams_map) {

		static bool is_playing;
		is_playing = false;
		FMOD_RESULT channel_result = FMOD_OK;

		if (stream.first != nullptr)
			channel_result = stream.first->isPlaying(&is_playing);

		if (stream.first == nullptr
			|| channel_result == FMOD_ERR_CHANNEL_STOLEN
			|| channel_result == FMOD_ERR_INVALID_HANDLE
			|| channel_result != FMOD_OK)
		{
			dp_WARN("FMOD channel failed because: {}", FMOD_ErrorString(channel_result));
			is_playing = false;
		}

		if (is_playing == false)
		{
			if (stream.first != nullptr)
				stream.first->stop();

			if (stream.second != nullptr)
				stream.second->release();

			active_streams_map.erase(stream.first);// s_active_streams.erase(iterator);
		}
	}
}




bool DaftPunk::Player::PlaySound_2D_FromFile(std::filesystem::path* p_file_path, FMOD_MODE p_mode)
{
	if (AllIsGood() != 0)
		return false;

	if (p_file_path->empty() 
		|| std::filesystem::exists(*p_file_path) == false
		|| std::filesystem::is_regular_file(*p_file_path) == false)
	{
		dp_WARN("Yo... How the hell do you think I'm supposed to magically know what sound to play? BRUH.ogg ????????\ngive me a proper std::filesystem::path");
		return false;
	}

	FMOD::Sound* sound = nullptr;
	FMOD_RESULT result;
	result = system_instance->createSound(p_file_path->string().c_str(), p_mode, nullptr, &sound);
	if (result != FMOD_OK)
	{
		dp_ERROR("Huge fuck up playing {} because\n{}", p_file_path->string(), FMOD_ErrorString(result));
		return false;
	}
	result = sound->setMode(p_mode);

	FMOD::Channel* channel = 0;
	if (result != FMOD_OK)
	{
		dp_ERROR("HOW THE HELL DID SETTING THE MODE FOR OUR SOUND CAUSE AN ERROR???? CONFUSION!");
		return false;
	}

	result = system_instance->playSound(sound, 0, false, &channel);
	if (result != FMOD_OK)
	{
		dp_ERROR("We cocked up playing the sound!");
		sound->release();
		return false;
	}
	//std::scoped_lock lock(mutex_stream);
	//active_streams_map[channel] = sound;

	dp_INFO("Streaming sound {}", p_file_path->string());
	return true;
}

bool DaftPunk::Player::PlaySound_2D_FromFile(std::filesystem::path p_file_path, FMOD_MODE p_mode)
{
	return PlaySound_2D_FromFile(&p_file_path, p_mode);
}


















/*
// ---------------------------------------------------------------------------------------
// Legacy Code
// TODO: Delete this XAudio2 crap
// ---------------------------------------------------------------------------------------
// Callback handlers, only implement the buffer events for maintaining play state
//
void DaftPunk::Engine::SoundCallbackHander::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
	// Triggers EVERY TICK
	//dp_TRACE("OnVoiceProcessingPassStart Triggered!");
}
void DaftPunk::Engine::SoundCallbackHander::OnVoiceProcessingPassEnd()
{
	// Triggers EVERY TICK
	//dp_TRACE("OnVoiceProcessingPassEnd Triggered!");
}
void DaftPunk::Engine::SoundCallbackHander::OnStreamEnd()
{
	dp_TRACE("OnStreamEnd Triggered!");
}
void DaftPunk::Engine::SoundCallbackHander::OnBufferStart(void* bufferContext)
{
	dp_TRACE("OnBufferStart Triggered!");
	m_isPlayingHolder = true;
}
void DaftPunk::Engine::SoundCallbackHander::OnBufferEnd(void* bufferContext)
{
	dp_TRACE("OnBufferEnd Triggered!");

	m_isPlayingHolder = false;
}
void DaftPunk::Engine::SoundCallbackHander::OnLoopEnd(void* bufferContext)
{
	dp_TRACE("OnLoopEnd Triggered!");
}
void DaftPunk::Engine::SoundCallbackHander::OnVoiceError(void* bufferContext, HRESULT error)
{
	dp_TRACE("OnVoiceError Triggered!");
}


HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;
}



HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}



HRESULT PrepFileForStream(std::filesystem::path* p_FilePath_ptr)
{
	auto strFileName = p_FilePath_ptr->wstring().c_str();
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	HANDLE hFile = CreateFileW(
		strFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		return HRESULT_FROM_WIN32(GetLastError());

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
		return S_FALSE;

	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
}

bool DaftPunk::Engine::Init(bool pForceInit)
{
	static HRESULT hr;

	bool all_is_legit = (SUCCEEDED(hr))
		&& m_Instance.GetAddressOf() != nullptr
		&& m_MasteringVoice != nullptr
		&& b_IsInitialised == true;

	if (all_is_legit && !pForceInit) {
		dp_INFO("We've been asked to Initialise our audio engine but that's already been done");
		return true;
	}

	else if (all_is_legit && pForceInit) {
		dp_INFO("We've been forced to Initialise the audio engine even though it's already been init'ted");
	}

	else if (!all_is_legit && pForceInit) {
		dp_WARN("We've been forced to reInitialise the audio engine but it was NEVER done properly the first time, wtf???");
	}
	else {
		dp_INFO("Starting up the engine!");
	}

	hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		dp_ERROR("::CoInitializeEx for XAudio Fucked up! returned code {}", hr);
		return false;
	}

	XAudio2Create(m_Instance.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (m_Instance.GetAddressOf() == nullptr) {
		dp_ERROR("XAudio2Create failed to make an XAudio Instance...");
		return false;
	}

	m_Instance.Get()->CreateMasteringVoice(&m_MasteringVoice);
	if (m_MasteringVoice == nullptr) {
		dp_ERROR("XAudio2 CreateMasteringVoice failed to 'the Voice'... Call Lady Jessica for help");
		return false;
	}

	b_IsInitialised = true;
	return true;
}



// TODO: WE NEED TO CLEAR THE DAMN BUFFER WHEN WE DONE
unsigned long DaftPunk::PlaySoundFromFile(std::filesystem::path p_FilePath)
{
	dp_TRACE("Going to try play {}", p_FilePath.string().c_str());
	WAVEFORMATEX wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	HANDLE hFile = CreateFileW(
		p_FilePath.wstring().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dp_ERROR("File supplied was invalid, INVALID_HANDLE_VALUE == hFile, got code {}", HRESULT_FROM_WIN32(GetLastError()));
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
	{
		dp_ERROR("File pointer supplied was invalid, INVALID_SET_FILE_POINTER got code {}", HRESULT_FROM_WIN32(GetLastError()));
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwChunkSize;
	DWORD dwChunkPosition;

	//check the file type, should be fourccWAVE or 'XWMA'
	dp_TRACE("Running FindChunk");
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);

	DWORD filetype;
	dp_TRACE("Running ReadChunkData");
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
	{
		dp_ERROR("{} is not a valid file type", p_FilePath.string());
		return S_FALSE;
	}

	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	DaftPunk::Engine::m_Instance->CreateSourceVoice(
		&DaftPunk::Engine::m_SourceVoice,
		&wfx,
		0,
		XAUDIO2_DEFAULT_FREQ_RATIO,
		reinterpret_cast<IXAudio2VoiceCallback*>(&DaftPunk::Engine::m_soundCallbackHandler),
		nullptr,
		nullptr
	);

	HRESULT hr = DaftPunk::Engine::m_SourceVoice->SubmitSourceBuffer(&buffer);
	dp_TRACE("New source buffer added");
	if (SUCCEEDED(hr))
	{
		dp_TRACE("Trying to start PLAY now");
		hr = DaftPunk::Engine::m_SourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
	}

	return true;
}
*/