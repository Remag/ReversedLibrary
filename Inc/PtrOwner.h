#pragma once
#include <Reassert.h>
#include <AllocationStrategy.h>
#include <MemoryUtils.h>
#include <MemoryOwner.h>
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

// Release function for virtual and common types.
template <class T>
typename Types::EnableIf<!Types::HasVirtualDestructor<T>::Result, void*>::Result getBaseOwnerPtr( T* ptr )
{
	return ptr;
}

template <class T>
typename Types::EnableIf<Types::HasVirtualDestructor<T>::Result, void*>::Result getBaseOwnerPtr( T* ptr )
{
	return dynamic_cast<void*>( ptr );
}

}	// namespace RelibInternal.

class CBaseObjectCreationFunction;
//////////////////////////////////////////////////////////////////////////

// Owning pointer.
// The pointer is deleted with MemoryManager::Free.
// Class-specific new and delete operators are not supported.
template <class Type, class Allocator>
class CPtrOwner : private CAllocationStrategy<Allocator> {
public:
	staticAssert( !Types::IsArray<Type>::Result );

	// Constructor. 
	// Create a null pointer.
	CPtrOwner( nullptr_t = nullptr );
	explicit CPtrOwner( Allocator& dynamicAllocator );
	// Move constructor. Copying is prohibited.
	CPtrOwner( CPtrOwner<Type, Allocator>&& other );

	// Constructor for moving from descendants.
	template <class Descendant>
	CPtrOwner( CPtrOwner<Descendant, Allocator>&& other );

	~CPtrOwner();

	const CPtrOwner<Type, Allocator>& operator=( nullptr_t );
	const CPtrOwner<Type, Allocator>& operator=( CPtrOwner&& other );
	template <class Descendant>
	const CPtrOwner<Type, Allocator>& operator=( CPtrOwner<Descendant, Allocator>&& other );

	// Destroy the object and nullify the pointer.
	void Release();

	bool IsNull() const;
	// Return the pointer. May return 0.
	Type* Ptr();
	const Type* Ptr() const;

	// Casting to the pointer type. 
	operator Type*();
	operator const Type*() const;
	// Dereferencing. Assert for 0.
	Type& operator*();
	const Type& operator*() const;
	// Member access. Assert for 0.
	Type* operator->();
	const Type* operator->() const;

	// CreateOwner needs access to owner pointer creation.
	template <class T, class AllocType, class... Args>
	friend CPtrOwner<T, AllocType> CreateOwner( Args&&... args );
	template <class T, class AllocType, class... Args>
	friend CPtrOwner<T, AllocType> AllocateOwner( AllocType& dynamicAllocator, Args&&... args );
	template <class ObjectType, class AllocType, class... CreationArgs>
	friend CPtrOwner<ObjectType, AllocType> CreateUniqueObject( const CBaseObjectCreationFunction* baseFunction, CreationArgs&&... args );
	template <class ObjectType, class AllocType, class... CreationArgs>
	friend CPtrOwner<ObjectType, AllocType> AllocateUniqueObject( const CBaseObjectCreationFunction* baseFunction, AllocType& allocator, CreationArgs&&... args );

	// Cast options need to be able to detach pointers.
	template <class Dest, class Src, class Allocator>
	friend CPtrOwner<Dest, Allocator> ptr_static_cast( CPtrOwner<Src, Allocator>&& src );
	template <class Dest, class Src, class Allocator>
	friend CPtrOwner<Dest, Allocator> ptr_dynamic_cast( CPtrOwner<Src, Allocator>&& src ); 

	// Base owner pointers need access to their descendant's detach method.
	template <class T, class Alloc>
	friend class CPtrOwner;

private:
	Type* ptr = nullptr;

	Type* detach();

	// Ptr is created with Allocator::Alloc. By default, ptr is created with the new expression.
	explicit CPtrOwner( Type* ptr );
	CPtrOwner( Type* ptr, Allocator& dynamicAllocator );

	// Copying is prohibited;
	CPtrOwner( CPtrOwner<Type, Allocator>& ) = delete;
	template <class OtherType, class OtherManager>
	CPtrOwner( CPtrOwner<OtherType, OtherManager>& ) = delete;

	void operator=( const CPtrOwner<Type, Allocator>& ) = delete;
	template <class OtherType, class OtherManager>
	void operator=( const CPtrOwner<OtherType, OtherManager>& ) = delete;
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::CPtrOwner( nullptr_t ) :
	CAllocationStrategy<Allocator>( CRawAllocationStrategyTag() ),
	ptr( nullptr )
{
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::CPtrOwner( Allocator& dynamicAllocator ) :
	CAllocationStrategy<Allocator>( dynamicAllocator )
{
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::CPtrOwner( Type* _ptr ) :
	ptr( _ptr )
{
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::CPtrOwner( Type* _ptr, Allocator& dynamicAllocator ) :
	CAllocationStrategy<Allocator>( dynamicAllocator ),
	ptr( _ptr )
{
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::CPtrOwner( CPtrOwner<Type, Allocator>&& other ) :
	CAllocationStrategy<Allocator>( move( other ) ),
	ptr( other.detach() )
{
}

template <class Type, class Allocator>
template<class Descendant>
CPtrOwner<Type, Allocator>::CPtrOwner( CPtrOwner<Descendant, Allocator>&& other ) :
	CAllocationStrategy<Allocator>( move( other ) ),
	ptr( other.detach() )
{
	staticAssert( ( Types::IsDerivedFrom<Descendant, Type>::Result ) );
	static_assert( Types::HasVirtualDestructor<Type>::Result, "Type erasure is not safe: the destructor is not virtual." );
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::~CPtrOwner()
{
	Release();
}

template <class Type, class Allocator>
const CPtrOwner<Type, Allocator>& CPtrOwner<Type, Allocator>::operator=( nullptr_t )
{
	Release();
	return *this;
}

template <class Type, class Allocator>
const CPtrOwner<Type, Allocator>& CPtrOwner<Type, Allocator>::operator=( CPtrOwner<Type, Allocator>&& other )
{
	Release();
	ptr = other.detach();
	CAllocationStrategy<Allocator>::operator=( move( other ) );
	return *this;
}

template <class Type, class Allocator>
template<class Descendant>
const CPtrOwner<Type, Allocator>& CPtrOwner<Type, Allocator>::operator=( CPtrOwner<Descendant, Allocator>&& other )
{
	staticAssert( ( Types::IsDerivedFrom<Descendant, Type>::Result ) );
	static_assert( Types::HasVirtualDestructor<Type>::Result, "Type erasure is not safe: the destructor is not virtual." );

	Release();
	ptr = other.detach();
	CAllocationStrategy<Allocator>::operator=( move( other ) );
	return *this;
}

template <class Type, class Allocator>
void CPtrOwner<Type, Allocator>::Release()
{
	Type* oldPtr = detach();
	if( oldPtr != nullptr ) {
		void* basePtr = RelibInternal::getBaseOwnerPtr( oldPtr );
		oldPtr->~Type();
		CAllocationStrategy<Allocator>::StrategyFree( basePtr );
	}
}

template <class Type, class Allocator>
Type* CPtrOwner<Type, Allocator>::detach()
{
	Type* result = ptr;
	ptr = nullptr;
	return result;
}

template <class Type, class Allocator>
bool CPtrOwner<Type, Allocator>::IsNull() const
{
	return ptr == nullptr;
}

template <class Type, class Allocator>
Type* CPtrOwner<Type, Allocator>::Ptr()
{
	return ptr;
}

template <class Type, class Allocator>
const Type* CPtrOwner<Type, Allocator>::Ptr() const
{
	return ptr;
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::operator Type*()
{
	return ptr;
}

template <class Type, class Allocator>
CPtrOwner<Type, Allocator>::operator const Type*() const
{
	return ptr;
}

template <class Type, class Allocator>
Type& CPtrOwner<Type, Allocator>::operator*()
{
	assert( ptr != nullptr );
	return *ptr;
}

template <class Type, class Allocator>
const Type& CPtrOwner<Type, Allocator>::operator*() const
{
	assert( ptr != nullptr );
	return *ptr;
}

template <class Type, class Allocator>
Type* CPtrOwner<Type, Allocator>::operator->()
{
	assert( ptr != nullptr );
	return ptr;
}

template <class Type, class Allocator>
const Type* CPtrOwner<Type, Allocator>::operator->() const
{
	assert( ptr != nullptr );
	return ptr;
}

//////////////////////////////////////////////////////////////////////////

// Owned object creation function. Creates the CPtrOwner using the new expression.
// Create an owner ptr with a static allocator. The allocator call operator new by default.
template <class ObjType, class Allocator = CRuntimeHeap, class... Args>
CPtrOwner<ObjType, Allocator> CreateOwner( Args&& ...args )
{
	staticAssert( !Types::IsArray<ObjType>::Result );
	CMemoryOwner<Allocator> memOwner( RELIB_STATIC_ALLOCATE( Allocator, sizeof( ObjType ) ) );
	ObjType* obj = ::new( memOwner.Ptr() ) ObjType( forward<Args>( args )... );
	memOwner.Detach();
	return CPtrOwner<ObjType, Allocator>( obj );
}

// Create an owner ptr with a given dynamic allocator.
template <class ObjType, class Allocator, class... Args>
CPtrOwner<ObjType, Allocator> AllocateOwner( Allocator& dynamicAllocator, Args&& ...args )
{
	staticAssert( !Types::IsArray<ObjType>::Result );
	CMemoryOwner<Allocator> memOwner( RELIB_ALLOCATE( dynamicAllocator, sizeof( ObjType ) ), dynamicAllocator );
	ObjType* obj = ::new( memOwner.Ptr() ) ObjType( forward<Args>( args )... );
	memOwner.Detach();
	return CPtrOwner<ObjType, Allocator>( obj, dynamicAllocator );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.