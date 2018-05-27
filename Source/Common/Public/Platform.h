#pragma once

#define MORE_WINDOWS 1

#if MORE_WINDOWS
	#undef WINAPI_FAMILY
	#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
	#define WIN32_LEAN_AND_MEAN
	#include "Windows.h"
#else
	#error Unsupported platform
#endif

