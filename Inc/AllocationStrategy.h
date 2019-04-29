#pragma once
#include <Allocator.h>
#include <Remath.h>
#include <TemplateUtils.h>
#include <RawBuffer.h>

namespace Relib {

// Flag for raw allocator initialization.
struct CRawAllocationStrategyTag {};
//////////////////////////////////////////////////////////////////////////

// Allocator must either have a static or dynamic allocation method. Allocation strategy for other allocators can't do anything.
template <class Allocator, class Enabled = void>
class CAllocationStrategy {
	static_assert( !Types::IsSame<Allocator, Allocator>::Result, "Allocator must have a CAllocator base class." );
};

// Specialization for static allocators.
template <class Allocator>
class CAllocationStrategy<Allocator, typename Types::EnableIf<Types::IsDerivedFrom<Allocator, TStaticAllocator>::Result>::Result> {
public:
	static_assert( Types::IsClass<Allocator>::Result, "Allocator must be a class" );

	CAllocationStrategy() = default;
	// A common constructor for all allocation strategies. Doesn't initialize the allocator.
	CAllocationStrategy( CRawAllocationStrategyTag ) {}

	// Unsized allocations.
	void* StrategyAllocate( int size )
		{ return Allocator::Allocate( size ); }
	void StrategyFree( void* buffer )
		{ return Allocator::Free( buffer ); }

	// Sized allocations.
	CRawBuffer StrategyAllocateSized( int size )
		{ return Allocator::AllocateSized( size ); }
	void StrategyFree( CRawBuffer buffer )
		{ return Allocator::Free( buffer ); }

#ifdef _DEBUG
	void* StrategyAllocate( int size, const char* fileName, int line )
		{ return Allocator::Allocate( size, fileName, line ); }
	CRawBuffer StrategyAllocateSized( int size, const char* fileName, int line )
		{ return Allocator::AllocateSized( size, fileName, line ); }
#endif
};

// Specialization for inline allocators.
template <class Allocator>
class CAllocationStrategy<Allocator, typename Types::EnableIf<Types::IsDerivedFrom<Allocator, TInlineAllocator>::Result>::Result> {
public:
	template <class... Args>
	explicit CAllocationStrategy( Args&&... allocatorArgs ) : allocator( forward<Args>( allocatorArgs )... ) {}
	// A common constructor for all allocation strategies. Doesn't initialize the allocator.
	CAllocationStrategy( CRawAllocationStrategyTag ) {}

	// Unsized allocations.
	void* StrategyAllocate( int size )
		{ return allocator.Allocate( size ); }
	void StrategyFree( void* ptr )
		{ return allocator.Free( ptr ); }

	// Sized allocations.
	CRawBuffer StrategyAllocateSized( int size )
		{ return allocator.AllocateSized( size ); }
	void StrategyFree( CRawBuffer buffer )
		{ return allocator.Free( buffer ); }

#ifdef _DEBUG
	void* StrategyAllocate( int size, const char* fileName, int line )
		{ return allocator.Allocate( size, fileName, line ); }
	CRawBuffer StrategyAllocateSized( int size, const char* fileName, int line )
		{ return allocator.AllocateSized( size, fileName, line ); }
#endif

private:
	Allocator allocator;
};

// Specialization for global allocators.
template <class Allocator>
class CAllocationStrategy<Allocator, typename Types::EnableIf<Types::IsDerivedFrom<Allocator, TGlobalAllocator>::Result>::Result> {
public:
	explicit CAllocationStrategy( Allocator& _allocator ) : allocator( &_allocator ) {}
	// A common constructor for all allocation strategies. Doesn't initialize the allocator.
	CAllocationStrategy( CRawAllocationStrategyTag ) : allocator( nullptr ) {}

	// Unsized allocations.
	void* StrategyAllocate( int size )
		{ return allocator->Allocate( size ); }
	void StrategyFree( void* ptr )
		{ return allocator->Free( ptr ); }

	// Sized allocations.
	CRawBuffer StrategyAllocateSized( int size )
		{ return allocator->AllocateSized( size ); }
	void StrategyFree( CRawBuffer buffer )
		{ return allocator->Free( buffer ); }

#ifdef _DEBUG
	void* StrategyAllocate( int size, const char* fileName, int line )
		{ return allocator->Allocate( size, fileName, line ); }
	CRawBuffer StrategyAllocateSized( int size, const char* fileName, int line )
		{ return allocator->AllocateSized( size, fileName, line ); }
#endif

private:
	Allocator* allocator;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

