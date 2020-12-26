#pragma once
#include <PtrCastUtils.h>
#include <SafeCounters.h>

// Shared pointers and weak pointers.
namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Pointer to an object with a reference counter. 
template <class Type, class Allocator /*= CRuntimeHeap*/>
class CSharedPtr {
public:
	CSharedPtr( nullptr_t = nullptr ) {}
	CSharedPtr( const CSharedPtr<Type, Allocator>& ptr );
	CSharedPtr( CSharedPtr<Type, Allocator>&& ptr );

	// New pointer creation.
	static CSharedPtr<Type, Allocator> CreateNewPtr( Type* newPtr, void* counterPtr );
	static CSharedPtr<Type, Allocator> CreateNewPtr( Type* newPtr, void* counterPtr, Allocator& allocator );

	// Constructor for copying and moving from descendants.
	template <class Descendant>
	CSharedPtr( const CSharedPtr<Descendant, Allocator>& other );
	template <class Descendant>
	CSharedPtr( CSharedPtr<Descendant, Allocator>&& other );

	~CSharedPtr();

	// Set the object pointer to zero and decrease the counter.
	void Release();

	// Check if the given ptr has a reference count of one.
	bool IsUnique() const;

	// Get the pointer. Result can be null.
	Type* Ptr()
		{ return ptr; }
	const Type* Ptr() const
		{ return ptr; }
	operator Type*() const
		{ return ptr; }

	// Dereference the pointer. Assertion fails if the pointer is null.
	Type& operator*() const;
	Type* operator->() const;

	// Assignment.
	const CSharedPtr<Type, Allocator>& operator=( CSharedPtr<Type, Allocator> other );
	template <class Descendant>
	const CSharedPtr<Type, Allocator>& operator=( CSharedPtr<Descendant, Allocator> other );

	// Hashing.
	int HashKey() const;

	template <class T, class A>
	friend class CSharedPtr;

	template <class Dest, class Src, class Alloc>
	friend CSharedPtr<Dest, Alloc> Relib::ptr_static_cast( CSharedPtr<Src, Alloc> src );
	template <class Dest, class Src, class Alloc>
	friend CSharedPtr<Dest, Alloc> Relib::ptr_dynamic_cast( CSharedPtr<Src, Alloc> src );

private:
	RelibInternal::CSafeStrongRefCounter<Allocator>* counterPtr = nullptr;
	Type* ptr = nullptr;

	CSharedPtr( Type* newType, void* counterPtr );
	CSharedPtr( Type* newType, void* counterPtr, Allocator& allocator );
};

//////////////////////////////////////////////////////////////////////////

template <class Type, class Allocator>
CSharedPtr<Type, Allocator>::CSharedPtr( const CSharedPtr<Type, Allocator>& other ) :
	ptr( other.ptr ),
	counterPtr( other.counterPtr )
{
	if( counterPtr != nullptr ) {
		counterPtr->AddStrongRef();
	}
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator>::CSharedPtr( CSharedPtr<Type, Allocator>&& other ) :
	ptr( other.ptr ),
	counterPtr( other.counterPtr )
{
	other.ptr = nullptr;
	other.counterPtr = nullptr;
}

template <class Type, class Allocator>
template <class Descendant>
CSharedPtr<Type, Allocator>::CSharedPtr( const CSharedPtr<Descendant, Allocator>& other ) :
	ptr( other.ptr ),
	counterPtr( other.counterPtr )
{
	staticAssert( ( Types::IsDerivedFrom<Descendant, Type>::Result ) );
	if( counterPtr != nullptr ) {
		counterPtr->AddStrongRef();
	}
}

template <class Type, class Allocator>
template <class Descendant>
CSharedPtr<Type, Allocator>::CSharedPtr( CSharedPtr<Descendant, Allocator>&& other ) :
	ptr( other.Ptr() ),
	counterPtr( other.counterPtr )
{
	staticAssert( ( Types::IsDerivedFrom<Descendant, Type>::Result ) );
	other.ptr = nullptr;
	other.counterPtr = nullptr;
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator>::CSharedPtr( Type* newPtr, void* strongCounterPtr ) :
	ptr( newPtr )
{
	assert( newPtr != nullptr );
	assert( strongCounterPtr != nullptr );
	counterPtr = ::new( strongCounterPtr ) RelibInternal::CSafeStrongRefCounter<Allocator>();
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator>::CSharedPtr( Type* newType, void* refCounterPtr, Allocator& allocator ) :
	ptr( newType )
{
	assert( newType != nullptr );
	assert( refCounterPtr != nullptr );
	counterPtr = ::new( refCounterPtr ) RelibInternal::CSafeStrongRefCounter<Allocator>( allocator );
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator> CSharedPtr<Type, Allocator>::CreateNewPtr( Type* newPtr, void* strongCounterPtr )
{
	return CSharedPtr<Type, Allocator>( newPtr, strongCounterPtr );
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator> CSharedPtr<Type, Allocator>::CreateNewPtr( Type* newPtr, void* counterPtr, Allocator& allocator )
{
	return CSharedPtr<Type, Allocator>( newPtr, counterPtr, allocator );
}

template <class Type, class Allocator>
CSharedPtr<Type, Allocator>::~CSharedPtr()
{
	Release();
}

template <class Type, class Allocator>
void CSharedPtr<Type, Allocator>::Release()
{
	Type* temp = ptr;
	if( counterPtr != nullptr ) {
		assert( temp != nullptr );
		// The pointer is nullified before the object is destroyed for exception safety.
		auto& counter = *counterPtr;
		ptr = nullptr;
		counterPtr = nullptr;
		if( counter.ReleaseStrongRef() ) {
			temp->~Type();
			counter.Expire();
		}
	}
}

template <class Type, class Allocator>
bool CSharedPtr<Type, Allocator>::IsUnique() const
{
	return counterPtr == nullptr || counterPtr->GetRefCount() == 1;
}

template <class Type, class Allocator>
Type& CSharedPtr<Type, Allocator>::operator*() const
{
	assert( ptr != nullptr );
	return *ptr;
}

template <class Type, class Allocator>
Type* CSharedPtr<Type, Allocator>::operator->() const
{
	assert( ptr != nullptr );
	return ptr;
}

template <class Type, class Allocator>
const CSharedPtr<Type, Allocator>& CSharedPtr<Type, Allocator>::operator=( CSharedPtr<Type, Allocator> other )
{
	swap( this->ptr, other.ptr );
	swap( this->counterPtr, other.counterPtr );
	return *this;
}

template <class Type, class Allocator>
template <class Descendant>
const CSharedPtr<Type, Allocator>& CSharedPtr<Type, Allocator>::operator=( CSharedPtr<Descendant, Allocator> other )
{
	staticAssert( ( Types::IsDerivedFrom<Descendant, Type>::Result ) );
	return operator=( ptr_static_cast<Type>( move( other ) ) );
}

template <class Type, class Allocator>
int CSharedPtr<Type, Allocator>::HashKey() const
{
	return static_cast<int>( reinterpret_cast<UINT_PTR>( ptr ) );
}

//////////////////////////////////////////////////////////////////////////

template <class ObjectType, class Allocator = CRuntimeHeap, class... ConstructorArgs>
CSharedPtr<ObjectType, Allocator> CreateShared( ConstructorArgs&&... args )
{
	const int counterOffset = CeilTo( sizeof( RelibInternal::CSafeStrongRefCounter<CRuntimeHeap> ), AllocatorAlignment );
	CMemoryOwner<Allocator> memOwner( RELIB_STATIC_ALLOCATE( Allocator, counterOffset + sizeof( ObjectType ) ) );
	BYTE* objectMemory = reinterpret_cast<BYTE*>( memOwner.Ptr() );
	ObjectType* newObject = ::new( objectMemory + counterOffset ) ObjectType( forward<ConstructorArgs>( args )... );
	memOwner.Detach();
	return CSharedPtr<ObjectType, Allocator>::CreateNewPtr( newObject, objectMemory );
}

template <class ObjectType, class Allocator, class... ConstructorArgs>
CSharedPtr<ObjectType, Allocator> AllocateShared( Allocator& allocator, ConstructorArgs&&... args )
{
	const int counterOffset = CeilTo( sizeof( RelibInternal::CSafeStrongRefCounter<Allocator> ), AllocatorAlignment );
	CMemoryOwner<Allocator> memOwner( RELIB_ALLOCATE( Allocator, counterOffset + sizeof( ObjectType ) ) );
	BYTE* objectMemory = reinterpret_cast<BYTE*>( memOwner.Ptr() );
	ObjectType* newObject = ::new( objectMemory + counterOffset ) ObjectType( forward<ConstructorArgs>( args )... );
	memOwner.Detach();
	return CSharedPtr<ObjectType, Allocator>::CreateNewPtr( newObject, objectMemory, allocator );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.