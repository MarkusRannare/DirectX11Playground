#pragma once

namespace MoGET
{
	// Custom array class as it's a hell to export std::vector out of dlls
	template<typename T> 
	class Array
	{
		Array() :
			Data( nullptr ),
			UsedEntries( 0 ),
			AllocatedEntries( 0 )
		{
		}

		template<typename T>
		Array( const Array<T>& Other ) :
			UsedEntries( Other.UsedEntries ),
			AllocatedEntries( Other.AllocatedEntries )
		{
		}

		protected:
			T*	Data;
			int UsedEntries;
			int AllocatedEntries;
	};
}
