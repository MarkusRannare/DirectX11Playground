#pragma once

#include <new>
#include <cassert>

// Disable varning of unused parameter, else destructor call fails on POD-types
#pragma warning( disable: 4189 )

// Ease up syntax for using the placement new with templates to copy elements in the array
#define PLACEMENT_COPY( TARGET, SOURCE ) \
	new( TARGET ) T( SOURCE )

namespace MoGET
{
	/**
	 * Standard Array class supporting storage of elements sequentially
	 */
	template<class T>
	class Array
	{
		public:
			static const int IndexNone = -1;
			/**
			 * Standard iterator implementation for iterating through the array if elements with some type-checking
			 */
			class Iterator
			{
				public:
					/**
					 * Checked operator, asserts if trying to dereference the Iterator out of bounds
					 *
					 * \return Referents to the element we currently pointing to
					 */
					T& operator*() const
					{
						assert( mIndex < mOuter.mNumItems && "Iterator is trying to access array out of bounds" );
						return mOuter[mIndex];
					}
					
					/**
					 * Checked operator, asserts if trying to use -> on the Iterator out of bounds
					 * 
					 * \return Referents to the element we currently trying to access
					 */
					T& operator->()
					{
						assert( mIndex < mOuter.mNumItems && "Iterator is trying to access array out of bounds" );
						return *mOuter[mIndex];
					}

					/**
					 * Steps forward the iterator and returns a iterator to the new location (itself)
					 *
					 * \return reference to this iterator
					 */
					Iterator& operator++()
					{
						++mIndex;

						return *this;
					}

					/**
					 * Steps forward the iterator a element and returns a iterator to the element before itself, only use when
					 * needed as it creates a element on the stack
					 *
					 * \return a new Iterator to the element we was pointing to before the operation
					 */
					Iterator operator++( int )
					{
						Iterator Tmp( mOuter, mIndex++ );

						return Tmp;
					}

					/**
					 * Steps back the iterator a element and returns a iterator to the new location (itself)
					 *
					 * \return reference to this iterator
					 */
					Iterator& operator--()
					{
						--mIndex;

						return *this;
					}

					/**
					 * Steps back the iterator a element and returns a iterator to the element before itself, only use when
					 * needed as it creates a element on the stack
					 *
					 * \return a new Iterator to the element we was pointing to before the operation
					 */
					Iterator operator--( int )
					{
						Iterator Tmp( mOuter, mIndex-- );

						return Tmp;
					}

					/**
					* \param Other the iterator to compare with
					 * \return true if the iterator is iterating over the same Array and is on the same element
					 */
					bool operator==( const Iterator& Other ) const
					{
						return &mOuter == &(Other.mOuter) && mIndex == Other.mIndex;
					}

					/**
					 * \param Other the iterator to compare with
					 * \return true if the iterator is either iterator over a different Array or we are pointing to different elements
					 */
					bool operator!=( const Iterator& Other ) const
					{
						return &mOuter != &(Other.mOuter) || mIndex != Other.mIndex;
					}

					/**
					 * Assigns this iterator a new value of another iterator
					 *
					 * \param Other the operator we should assign values from
					 * \return a reference to the new iterator
					 */
					Iterator& operator=( const Iterator& Other )
					{
						assert( &mOuter == &(Other.mOuter) && "Can't assing a iterator with a new outer Array! Then we need to change the member from a & to a *" );
						
						if( this != &Other )
						{
							mOuter = Other.mOuter;
							mIndex = Other.mIndex;
						}

						return *this;
					}

					/**
					 * Copies the iterator from another iterator
					 *
					 * \param Other the Iterator to copy
					 */
					Iterator( const Iterator& Other ) :
						mOuter( Other.mOuter ),
						mIndex( Other.mIndex )
					{
					}
				private:
					friend class Array<T>;
					/**
					 * Private constructor ised internally to create this iterator
					 *
					 * \param Outer the array we are iterating over
					 * \param Index the value we are starting it, might not be a valid index
					 */
					Iterator( Array<T>& Outer, int Index ) :
						mOuter( Outer ),
						mIndex( Index )
					{
					}
				private:
					Array<T>&	mOuter;
					size_t		mIndex;
			};
		public:
			/**
			 * Standard constructor, assumes that Size is > 0
			 *
			 * \param Size the number of elements to preallocate in the Array
			 */
			Array( int Size = 4 ) :
				mAllocatedItems( Size ),
				mNumItems( 0 ),
				mData( (T*)malloc( sizeof(T) * Size ) )
			{
				assert( Size != 0 && "Can't allocate a array with size 0" );
			}

			// @todo: Ability to initialize Array with initialization list

			/**
			 * Copy constructor, will fail (and assert) if we run out of memory
			 * Will call the copy-constructor to copy the elements from the other Array
			 *
			 * \param Other the array to copy
			 */
			Array( const MoGET::Array<T>& Other ) :
				mAllocatedItems( Other.mAllocatedItems ),
				mNumItems( Other.mNumItems ),
				mData( (T*)appMalloc( sizeof(T) * Other.mAllocatedItems ) )
			{
				assert( mData != NULL && "Out of memory" );

				// Loops instead of memcpy to make sure get copy operator called on the new objects
				for( size_t Idx = 0; Idx < mNumItems; ++Idx )
				{
					// Copy the elements to the new array
					PLACEMENT_COPY( &mData[Idx], Other[Idx] );
				}
			}

			/**
			 * Assignment operator will fail (and assert) if we run out of memory
			 * First calls destructor on all items in Array, the call the copy-constructor 
			 * to copy the elements from the other Array
			 *
			 * \param Other the array to copy
			 * \return A referece to the instance to this array
			 */
			Array<T>& operator=( const Array<T>& Other )
			{
				// Just do the heavy lifting if we are ain't trying to assing to ourself
				if( this != &Other )
				{
					// Make sure all destructors are run on the remaining objects!
					for( int Idx = mNumItems - 1; Idx >= 0; --Idx )
					{
						RemoveAt( Idx );
					}

					// If we already are at correct size, to allocate new memory
					if( mAllocatedItems != Other.mAllocatedItems)
					{
						appFree( mData );

						T* NewData = (T*)appMalloc( sizeof(T) * Other.mAllocatedItems);
						assert( mData != NULL && "Out of memory" );
						mAllocatedItems = Other.mAllocatedItems;
					}

					mNumItems = Other.mNumItems;

					// This instead of memcpy to make sure get assignment/cpy operator called on the new objects
					for( size_t Idx = 0; Idx < mNumItems; ++Idx )
					{
						// Not using AddItem to remove size-checking ( we know we have allocated enough memory already )
						PLACEMENT_COPY( &mData[Idx], Other[Idx] );
					}
				}

				return *this;
			}

			/**
			 * Calls the destructor on all elements in the array and frees the memory of our data
			 */
			~Array()
			{
				// @todo: Optimize this, it should be enough to do mData[Idx].~T();, but I want unit testing before
				// I rewrite that
				// Make sure all destructors are run on the remaining objects!
				for( int Idx = (int)mNumItems - 1; Idx >= 0; --Idx )
				{
					RemoveAt( Idx );
				}

				free( mData );
			}

			/**
			 * \return true if both arrays contain the same number of items and all items are equal
			 * \note Order of elements matter
			 */
			bool operator==( const Array<T>& Other ) const
			{
				if( mNumItems != Other.mNumItems )
				{
					return false;
				}
				
				for( int Idx = mAllocatedItems - 1; Idx >= 0; --Idx )
				{
					if( mData[Idx] != Other.mData[Idx] )
					{
						return false;
					}
				}
				return true;
			}

			/**
			 * \return true if the number of elements differ in the array, or any of the elements are different
			 * \note Order of elements matter
			 */
			bool operator!=( const Array<T>& Other ) const
			{
				return !(Other == *this);
			}

			/**
			 * Adds a item at the end of the Array, and resizes the array if it needs too
			 * \note Copy Ctor is used to initialize the element at the end of the array
			 * \param Item the Item that is placed at the end of the array
			 */
			void AddItem( const T& Item )
			{
				assert( mNumItems <= mAllocatedItems && "More items than size, threading error?"  );
				if( mNumItems == mAllocatedItems )
				{
					Resize( mAllocatedItems * 2 + 1 );
				}

				PLACEMENT_COPY( &mData[mNumItems++], Item );
			}
			
			/**
			 * Removes the first item it finds that is comparable with the item
			 * \param Item The item to remove
			 **/
			void RemoveItem( const T& Item )
			{
				// If not found, the removal loop won't run
				size_t FoundIdx = mNumItems;
				for( size_t Idx = 0; Idx < mNumItems; ++Idx )
				{
					if( mData[Idx] == Item )
					{
						// Destroy the object if it's on the stack! @TODO: Verify this
						FoundIdx = Idx;
						break;
					}
				}

				if( FoundIdx < mNumItems )
				{
					RemoveAt( FoundIdx );
				}
			}

			/**
			 * Removes item at the given index (also destructs it)
			 * \note Assignment operator is used to move the other objects in the array
			 *
			 * \param AtIndex The index we want to remove a item from
			 */
			void RemoveAt( size_t AtIndex )
			{
				assert( AtIndex < mNumItems && "You cant remove a item that's outside the arrays bounds" );

				T& ItemToRemove = mData[AtIndex];
				ItemToRemove.~T();

				for( size_t Idx = AtIndex + 1; Idx < mNumItems; ++Idx )
				{
					PLACEMENT_COPY( &mData[Idx-1], mData[Idx] );
					//mData[Idx-1] = mData[Idx];
				}

				--mNumItems;
			}

			// @todo: Remove/RemoveSwap/RemoveOne with lambda

			/**
			 * Iterator version that removes a item that a iterator is pointing to
			 * 
			 * \param Itr a iterator that specifies where we want to remove a item
			 */
			inline void RemoveAt( Iterator& Itr )
			{
				RemoveAt( Itr.mIndex );
			}

			/**
			 * Gets a item and checks the bounds so that you don't access a item you haven't added
			 *
			 * \param Idx The index where to access the array
			 * \return T& A reference to the item
			 **/
			T& operator[]( size_t Idx )
			{
				assert( Idx < mNumItems && "Trying to access array out of bounds" );

				return mData[Idx];
			}

			/**
			 * Gets a item and checks the bounds so that you don't access a item you haven't added
			 *
			 * \param Idx The index where to access the array
			 * \return T& A reference to the item
			 **/
			const T& operator[]( size_t Idx ) const
			{
				assert( Idx < mNumItems && "Trying to access array out of bounds" );

				return mData[Idx];
			}

			/**
			 * @TODO: CHECK IF THIS FEELS RIGHT o.O
			 * Adds a zeroed out element and returns the index to the element
			 *
			 * \return int The index of the array where the element was added!
			 **/
			int AddZeroed()
			{
				if( mNumItems == mAllocatedItems )
				{
					Resize( mAllocatedItems * 2 + 1 );
				}
				T& Data = mData[mNumItems++];
				memset( &Data, 0, sizeof(T) );

				return mNumItems - 1;
			}

			/**
			 * \return the number of items currently in the array
			 */
			size_t Length() const
			{
				return mNumItems;
			}

			/**
			 * Clears all items in the array, and also calls the constructor on them
			 */
			void Clear()
			{
				for( int Idx = mNumItems - 1; Idx >= 0; --Idx )
				{
					RemoveAt( Idx );
				}
				assert( mNumItems == 0 && "Something is wrong, we have item's left after a Clear o.O" );
			}

			// @todo: Find with lamdbda statments
			// 

			/**
			 * Finds a item in the array and returns the index where in the array the item is placed
			 *
			 * \param Item the item to find the index of
			 * \return IndexNone if we don't find the item, othervise it's index
			 */
			int Find( const T& Item ) const
			{
				for( size_t Idx = 0; Idx != mNumItems; ++Idx )
				{
					if( mData[Idx] == Item )
					{
						return Idx;
					}
				}
				return IndexNone;
			}

			/**
			 * \see Find( const T& Item )
			 */
			inline int Find( const T&& Item ) const
			{
				return Find( Item );
			}

			/**
			 * Resizes the array to the specified new size, fails if we tries to resize it to a smaller size than the number of elements already in the array
			 * \note Uses the copy constructor to initialize the elements in the array
			 *
			 *	\param NewSize the new size of the Array
			 */
			void Resize( size_t NewSize )
			{
				// @todo: Remove this and destroy elements instead
				assert( NewSize >= mNumItems && "Can't resize array to less size than there is items in the array" );

				T* NewData = (T*)malloc( NewSize * sizeof(T) );
				assert( NewData != NULL && "Out of memory on resize of array" );

				for( size_t Idx = 0; Idx < mNumItems; ++Idx )
				{
					// Copy old instance to new location
					PLACEMENT_COPY( &NewData[Idx], mData[Idx] );
					// Destroy the old instance
					mData[Idx].~T();
				}

				free( mData );
				mData = NewData;
				mAllocatedItems = NewSize;
			}

			/**
			 * Set the number of elements in the array, potentially growing/shrinking the array
			 * They can optionally don't be initialized
			 * \param NumItems Number of Items we want in the array
			 * \param Initialize If true, then we run the default constructor on the items
			 * 
			 */
			void SetNum( size_t NumItems, bool Initialize = true )
			{
				// Less items than there is in the array, destroy the ones that's too much of
				if( mNumItems == NumItems )
				{
					// Nothing to do
					return;
				}
				if( NumItems < mNumItems )
				{
					for( size_t Idx = NumItems - 1; Idx < mNumItems; ++Idx )
					{	
						mData[Idx].~T();
					}
					mNumItems = NumItems;
					return;
				}
				else
				{
					// If there is a need for more elements, allocate them
					if( NumItems >= mAllocatedItems )
					{
						Resize( NumItems );
					}
					if( Initialize )
					{
						for( size_t Idx = mNumItems; Idx < mAllocatedItems; ++Idx )
						{
							// Run constructor on the new items
							new( &mData[Idx] ) T();
						}
					}
					mNumItems = NumItems;
				}
			}

			/**
			 * Get a raw data pointer to the internal array
			 * \return a unchecked pointer to the internal array, might get invalidated at any time
			 */
			const T* GetRawData() const
			{
				return mData;
			}

			/**
			 * Get a raw data pointer to the internal array
			 * \return a unchecked pointer to the internal array, might get invalidated at any time
			 */
			T* GetRawData()
			{
				return mData;
			}

			/**
			 * \return returns a iterator pointing to the first element of the array
			 */
			Iterator First()
			{
				return Iterator( *this, 0 );
			}
			
			/**
			 * \return returns a iterator pointing to the element after the last in the array
			 */
			Iterator Last()
			{
				return Iterator( *this, mNumItems );
			}
		private:
			/** The number of items that's allocated space for */
			size_t	mAllocatedItems;
			/** Number of items that's inserted into the array */
			size_t	mNumItems;
			/** The raw data the elements is stored in */
			T*		mData;
	};
}

#pragma warning( default: 4189 )
