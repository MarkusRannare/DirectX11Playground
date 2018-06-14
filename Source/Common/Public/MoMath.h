#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

namespace MoRE
{
	template<typename T>
	T Clamp(T Val, T MinVal, T MaxVal)
	{
		return max(min(Val, MaxVal), MinVal);
	}

	inline UINT ArgbToAbgr( UINT Argb )
	{
		BYTE A = (Argb >> 24)	& 0xff;
		BYTE R = (Argb >> 16)	& 0xff;
		BYTE G = (Argb >> 8)	& 0xff;
		BYTE B = (Argb >> 0)	& 0xff;

		return (A << 24) | (B << 16) | (G << 8) | (R << 0);
	}
}

