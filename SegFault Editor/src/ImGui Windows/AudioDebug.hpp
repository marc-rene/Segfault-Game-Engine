#include "imgui.h"
#include "DaftPunk.hpp"


void ShowAudioDebugWindow()
{
	ImGui::Begin("Detective Punk");
	/*
	for (const std::pair<FMOD::Channel*, FMOD::Sound*> channelSound : DaftPunk::Player::active_streams_map)
	{
		int channelID;
		channelSound.first->getIndex(&channelID);
		char* soundName = (char*)malloc(64);
		channelSound.second->getName(soundName, 63);

		ImGui::Text("Channel %d playing sound: %s", channelID, soundName);
	}
	*/

	ImGui::End();
}
