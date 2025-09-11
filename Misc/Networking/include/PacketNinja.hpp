#pragma once
#include "Tout.hpp"

int LINK_TEST_PacketNinja();

#define pn_NAME "Packet Ninja       "	
#define pn_NAME_ns "Packet_Ninja"

#define pn_TRACE(...)	TRACE	(pn_NAME, __VA_ARGS__)
#define pn_INFO(...)	INFO	(pn_NAME, __VA_ARGS__)
#define pn_WARN(...)	WARN	(pn_NAME, __VA_ARGS__)
#define pn_ERROR(...)	OhSHIT	(pn_NAME, __VA_ARGS__)