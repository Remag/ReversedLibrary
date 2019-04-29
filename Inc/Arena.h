#pragma once
#include <Redefs.h>
#include <BaseStackAllocator.h>
#include <RawBuffer.h>

namespace Relib {

template <class PageAllocator, class PageSizeStrategy>
class CArenaStateOwner;
//////////////////////////////////////////////////////////////////////////

// Class for unsafe allocations.
// All the memory is placed on a stack. After it is not needed anymore, the whole stack is destroyed without calling destructors.
template <class PageAllocator = CVirtualPageAllocator, class PageStrategy = CStaticByteResizeStrategy<64 * 1024>>
class CArena : public TGlobalAllocator {
public:
	CArena() = default;
	CArena( CArena<PageAllocator, PageStrategy>&& other ) = default;
	CArena& operator=( CArena&& other ) = default;

	typedef PageAllocator TPageAllocator;
	typedef PageStrategy TPageSizeStrategy;
	typedef CStackState<PageAllocator, PageStrategy> TState;
	typedef CArenaStateOwner<PageAllocator, PageStrategy> TStateOwner;

	// Free all the memory.
	void FreePages()
		{ return baseAllocator.FreePages(); }

	// Forget about the used memory and prepare to reuse it.
	void Reset()
		{ return baseAllocator.ResetWithoutDump(); }
	// Reset to the given state.
	void Reset( const TState& state )
		{ return baseAllocator.ResetWithoutDump( state ); }

	// Remember current state to be able to reset to it later.
	TState GetState() const
		{ return baseAllocator.GetState(); }
	// Create a state that restores itself on destruction. The arena must not be moved until the state is destroyed.
	TStateOwner PushState() const
		{ return TStateOwner( *this ); }

	// Copy a given value on the stack.
	template <class T>
	T& Copy( T value );
	// Construct a new value using given arguments.
	template <class T, class... Args>
	T& Create( Args&&... args );

	// Create a raw buffer.
	CRawBuffer Create( int size, int alignment );

	// Create a value with defined destructor. The caller takes full responsibility for cleaning up the value.
	template <class T, class... Args>
	T& CreateDestructibleValue( Args&&... args );

	// Allocation methods.
	void* AllocateAligned( int size, int alignment );
	void* Allocate( int size );
	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer buffer );
	void Free( void* buffer );

#ifdef _DEBUG
	void* Allocate( int size, const char* fileName, int line );
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
#endif

private:
	RelibInternal::CBaseStackAllocator<TPageAllocator, TPageSizeStrategy> baseAllocator;

	// Copying is prohibited.
	CArena( const CArena& ) = delete;
	void operator=( const CArena& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Owning stack state that pops itself on destruction.
template <class PageAllocator, class PageSizeStrategy>
class CArenaStateOwner {
public:
	CArenaStateOwner( CArena<PageAllocator, PageSizeStrategy>& _arena ) : arena( _arena ), state( _arena.GetState() ) {}
	~CArenaStateOwner()
		{ arena.Reset( state ); }

private:
	CArena<PageAllocator, PageSizeStrategy>& arena;
	CStackState<PageAllocator, PageSizeStrategy> state;

	// Copying is prohibited.
	CArenaStateOwner( CArenaStateOwner& ) = delete;
	void operator=( CArenaStateOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class PageAllocator, class PageStrategy>
template <class T>
T& CArena<PageAllocator, PageStrategy>::Copy( T value )
{
	return Create<T>( move( value ) );
}

template <class PageAllocator, class PageStrategy>
template <class T, class... Args>
T& CArena<PageAllocator, PageStrategy>::Create( Args&&... args )
{
	static_assert( Types::HasTrivialDestructor<T>::Result, "Storing this type in arena is not safe: the destructor will not be called." );
	return CreateDestructibleValue<T>( forward<Args>( args )... );
}

template <class PageAllocator, class PageStrategy>
CRawBuffer CArena<PageAllocator, PageStrategy>::Create( int size, int alignment )
{
	return CRawBuffer( baseAllocator.Allocate( size, alignment ), size );
}

template <class PageAllocator, class PageStrategy>
template <class T, class... Args>
T& CArena<PageAllocator, PageStrategy>::CreateDestructibleValue( Args&&... args )
{
	BYTE* rawResult = baseAllocator.Allocate( sizeof( T ), alignof( T ) );
	assert( reinterpret_cast<size_t>( rawResult ) % alignof( T ) == 0 );
	return *::new( rawResult ) T( forward<Args>( args )... );
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void* CArena<PageAllocator, PageStrategy>::Allocate( int size )
{
	return baseAllocator.Allocate( size, AllocatorAlignment );
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void* CArena<PageAllocator, PageStrategy>::AllocateAligned( int size, int alignment )
{
	return baseAllocator.Allocate( size, alignment );
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
CRawBuffer CArena<PageAllocator, PageStrategy>::AllocateSized( int size )
{
	return CRawBuffer{ Allocate( size ), size };
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void CArena<PageAllocator, PageStrategy>::Free( CRawBuffer )
{
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void CArena<PageAllocator, PageStrategy>::Free( void* )
{
}

#ifdef _DEBUG
template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void* CArena<PageAllocator, PageStrategy>::Allocate( int size, const char*, int )
{
	return Allocate( size );
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
CRawBuffer CArena<PageAllocator, PageStrategy>::AllocateSized( int size, const char*, int )
{
	return AllocateSized( size );
}
#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

