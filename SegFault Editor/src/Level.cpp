#include "Level.hpp"

void Level::LINK_Test()
{
	INFOc("GREAT SUCESS");
}



entt::entity Level::AddActor()
{
	return this->LevelRegistry.create();
}