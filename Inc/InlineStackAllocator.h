#pragma once
#include <AllocationStrategy.h>
#include <StackArray.h>
#include <Allocator.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Allocator that returns local storage for every allocation that fits.
// Specified allocator is used for larger requests.
// Note that Allocate returns the same buffer for all small allocations. The caller is responsible for freeing the memory before a subsequent small allocation.
// Used by flexible arrays.
template <class Allocator, int byteSize, int alignment>
class CInlineStackAllocator : public TInlineAllocator {
public:
	CInlineStackAllocator() = default;
	CInlineStackAllocator( Allocator& allocator ) : strategyAndStack( allocator ) {}

	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer buffer );

	void* Allocate( int size );
	void Free( void* buffer );

#ifdef _DEBUG
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
	void* Allocate( int size, const char* fileName, int line );
#endif

private:
	// Compress the strategy into the stack.
	struct CStrategyAndStack : public CAllocationStrategy<Allocator> {
		alignas( alignment ) CStackArray<BYTE, byteSize> Stack;

		CStrategyAndStack() = default;
		explicit CStrategyAndStack( Allocator& allocator ) : CAllocationStrategy( allocator ) {}
	};
	CStrategyAndStack strategyAndStack;

	CAllocationStrategy<Allocator>& getStrategy()
		{ return strategyAndStack; }
	CStackArray<BYTE, byteSize>& getStack()
		{ return strategyAndStack.Stack; }

	// Copying is prohibited.
	CInlineStackAllocator( CInlineStackAllocator& ) = delete;
	void operator=( CInlineStackAllocator& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Allocator, int byteSize, int alignment>
CRawBuffer CInlineStackAllocator<Allocator, byteSize, alignment>::AllocateSized( int size )
{
	return size <= byteSize ? CRawBuffer{ getStack().Ptr(), byteSize } : getStrategy().StrategyAllocateSized( size );
}

template <class Allocator, int byteSize, int alignment>
void* CInlineStackAllocator<Allocator, byteSize, alignment>::Allocate( int size )
{
	return size <= byteSize ? getStack().Ptr() : getStrategy().StrategyAllocate( size );
}

template <class Allocator, int byteSize, int alignment>
void CInlineStackAllocator<Allocator, byteSize, alignment>::Free( void* buffer )
{	
	if( buffer != getStack().Ptr() ) {
		getStrategy().StrategyFree( buffer );
	}
}

template <class Allocator, int byteSize, int alignment>
void CInlineStackAllocator<Allocator, byteSize, alignment>::Free( CRawBuffer buffer )
{
	if( buffer.Ptr() != getStack().Ptr() ) {
		getStrategy().StrategyFree( buffer );
	}
}

#ifdef _DEBUG
template <class Allocator, int byteSize, int alignment>
CRawBuffer CInlineStackAllocator<Allocator, byteSize, alignment>::AllocateSized( int size, const char* fileName, int line )
{
	return size <= byteSize ? CRawBuffer{ getStack().Ptr(), byteSize } : getStrategy().StrategyAllocateSized( size, fileName, line );
}

template <class Allocator, int byteSize, int alignment>
void* CInlineStackAllocator<Allocator, byteSize, alignment>::Allocate( int size, const char* fileName, int line )
{
	return size <= byteSize ? getStack().Ptr() : getStrategy().StrategyAllocate( size, fileName, line );
}
#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

