#pragma once

#include "Common.h"

namespace MoRE
{
	COMMON_API bool ReadFileContent(const char* Filename, const char* ReadMode, char** out_FileContent, long& out_ContentSize);
}
