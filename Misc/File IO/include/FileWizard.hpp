#pragma once
#include "Tout.hpp"

int LINK_TEST_FileWizard();

#define fw_NAME		"File Wizard        "	
#define fw_NAME_ns	"File_Wizard"

#define fw_TRACE(...)	TRACE	(fw_NAME, __VA_ARGS__)
#define fw_INFO(...)	INFO	(fw_NAME, __VA_ARGS__)
#define fw_WARN(...)	WARN	(fw_NAME, __VA_ARGS__)
#define fw_ERROR(...)	OhSHIT	(fw_NAME, __VA_ARGS__)