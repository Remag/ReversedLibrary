#pragma once
#include <PtrOwner.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

template <class Dest, class Src, class Allocator>
CSharedPtr<Dest, Allocator> ptr_static_cast( CSharedPtr<Src, Allocator> src )
{
	// Swap pointers without touching object's reference counter.
	CSharedPtr<Dest, Allocator> result;
	result.ptr = static_cast<Dest*>( src.ptr );
	result.counterPtr = src.counterPtr;
	src.ptr = nullptr;
	src.counterPtr = nullptr;
	return move( result );
}

template <class Dest, class Src, class Allocator>
CSharedPtr<Dest, Allocator> ptr_dynamic_cast( CSharedPtr<Src, Allocator> src )
{
	CSharedPtr<Dest, Allocator> result;
	result.ptr = dynamic_cast<Dest*>( src.ptr );
	if( result.ptr != nullptr ) {
		src.ptr = nullptr;
		result.counterPtr = src.counterPtr;
		src.counterPtr = nullptr;
	}
	return move( result );
}

//////////////////////////////////////////////////////////////////////////

template<class Dest, class Src, class Allocator>
CPtrOwner<Dest, Allocator> ptr_static_cast( CPtrOwner<Src, Allocator>&& src )
{
	// Swap pointers without touching object's reference counter.
	return CPtrOwner<Dest, Allocator>( static_cast<Dest*>( src.detach() ) );
}

template<class Dest, class Src, class Allocator>
CPtrOwner<Dest, Allocator> ptr_dynamic_cast( CPtrOwner<Src, Allocator>&& src )
{
	// Swap pointers without touching object's reference counter.
	return CPtrOwner<Dest, Allocator>( dynamic_cast<Dest*>( src.detach() ) );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.