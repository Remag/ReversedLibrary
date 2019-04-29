#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Library allocator type.
// Static allocators are completely stateless and manage memory with static methods.
// Inline allocators are created and owned by every objects that uses them.
// Global allocators are owned by the user and are used by reference.
enum TAllocatorType {
	AT_Static,
	AT_Inline,
	AT_Global
};

// Base class for custom allocators. Contains compile-time type information.
template <TAllocatorType allocatorType>
class CAllocator {
public:
	static const TAllocatorType AllocatorType = allocatorType;
};

typedef CAllocator<AT_Static> TStaticAllocator;
typedef CAllocator<AT_Inline> TInlineAllocator;
typedef CAllocator<AT_Global> TGlobalAllocator;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

