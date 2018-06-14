#pragma once

namespace MoRE
{
	bool ReadFileContent(const char* Filename, const char* ReadMode, char** out_FileContent, long& out_ContentSize);
}
