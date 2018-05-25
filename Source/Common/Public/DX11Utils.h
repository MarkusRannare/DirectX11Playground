#pragma once

#include "DxErr.h"

#define ReleaseCOM(x) { if(x){ x->Release(); x = nullptr; } }

// @todo: Make dependent on other preprocessor flag
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)													\
	{															\
		HRESULT hr = (x);										\
		if(FAILED(hr))											\
		{														\
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);	\
		}														\
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 
