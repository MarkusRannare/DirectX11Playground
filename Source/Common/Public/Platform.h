#pragma once

#include "Build.h"

#if MOGET_WINDOWS
	#undef WINAPI_FAMILY
	#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
	#define WIN32_LEAN_AND_MEAN
	#include "Windows.h"
	#include "Windowsx.h"
#else
	#error Unsupported platform
#endif

