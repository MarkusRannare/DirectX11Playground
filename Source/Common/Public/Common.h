#pragma once

#include "Build.h"

#ifdef COMMON_EXPORTS
	#define COMMON_API DLL_EXPORT
#else
	#define COMMON_API DLL_IMPORT
#endif