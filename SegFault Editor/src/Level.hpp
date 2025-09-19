#pragma once

#include "entt/entt.hpp"
#include "Rendering.hpp"
#include "DirectXMath.h"



struct Transform
{
	DirectX::XMFLOAT3 Location;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
};

struct Level
{
	const std::string LevelName;
	std::filesystem::path LevelPath;
	entt::registry LevelRegistry;
	//static entt::registry GlobalRegistry; // TODO: Is this needed?

	Level(std::string p_levelName = "Untitled")
		: LevelName(p_levelName)
	{
		LevelRegistry = entt::registry();

	}


	static void LINK_Test();
	entt::entity AddActor();
	void update();

};

//------------------------------------------------------------------------------
// LES COMPONENTS
//------------------------------------------------------------------------------
struct comp_ScenePosition
{
	Transform LocalTransform;
};



struct comp_Camera
{
	// Calculate camera view matrix
	DirectX::XMFLOAT3 WorldLocation = { -10, 0, 0 };
	DirectX::XMFLOAT3 Rotation = { 0,0,0 };

	bool b_MainCamera; // Should our render target focus on THIS one?
	bool b_Orthographic;

	float NearClipPlane = 0.1;
	float FarClipPlane = 9000;


	void Update(float renderDeltaTime);
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectionMatrix(float fov, float aspectRatio, float nearPlane = 1.0f, float farPlane = 1000.0f);
	void Init(DirectX::XMFLOAT3 position);

};













//------------------------------------------------------------------------------
// LES SYSTEMS
//------------------------------------------------------------------------------