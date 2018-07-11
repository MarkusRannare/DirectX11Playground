#pragma once

#include <math.h>
#include <algorithm>

namespace MoGET
{
	namespace Math
	{
		const double Pi = 3.14159265358979323846;
		const double HalfPi = Pi / 2.0;
		const double QuarterPi = Pi / 4.0;

		template<typename T>
		T Min( T A, T B )
		{
			return A < B ? A : B;
		}

		template<typename T>
		T Max( T A, T B )
		{
			return A > B ? A : B;
		}

		template<typename T>
		T Clamp(T Val, T MinVal, T MaxVal)
		{
			return Max(Min(Val, MaxVal), MinVal);
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
}

