#include "MoFile.h"
#include <cstdio>

namespace MoRE
{
	bool ReadFileContent(const char* Filename, const char* ReadMode, char** out_FileContent, long& out_ContentSize)
	{
		FILE* FileHandle = nullptr;
		errno_t Error = fopen_s(&FileHandle, Filename, ReadMode);

		if (Error != 0 || FileHandle == nullptr)
		{
			*out_FileContent = nullptr;
			return false;
		}

		// Get size of file 
		fseek(FileHandle, 0, SEEK_END);
		out_ContentSize = ftell(FileHandle);
		rewind(FileHandle);

		char* ShaderBytecode = new char[out_ContentSize];
		fread(ShaderBytecode, 1, out_ContentSize, FileHandle);
		fclose(FileHandle);
		*out_FileContent = ShaderBytecode;

		return true;
	}
}