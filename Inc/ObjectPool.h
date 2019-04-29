#pragma once
#include <PersistentStorage.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Reference that returns itself to the pool on destruction.
template <class T, int objectGroupSize, class GroupAllocator, class GeneralAllocator>
class CPoolRef {
public:
	CPoolRef( CObjectPool<T, objectGroupSize, GroupAllocator, GeneralAllocator>& _pool, T& _object ) : pool( _pool ), object( &_object ) {}
	CPoolRef( CPoolRef&& other ) : object( other.object ) { other.object = nullptr; }

	~CPoolRef();

	T& Value()
		{ return *object; }
	
private:
	CObjectPool<T, objectGroupSize, GroupAllocator, GeneralAllocator>& pool;
	T* object;
};

//////////////////////////////////////////////////////////////////////////

// Container for reusable objects.
// Objects are default-constructed on demand.
// Freed objects are not destroyed and can be returned again in the same state.
template <class T, int objectGroupSize, class GroupAllocator = CRuntimeHeap, class GeneralAllocator = CRuntimeHeap>
class CObjectPool {
public:
	typedef CPoolRef<T, objectGroupSize, GroupAllocator, GeneralAllocator> TPoolRef;

	CObjectPool() = default;

	auto GetOrCreate();
	void Release( T& object );

private:
	struct CPoolFreeObject : public T {
		CPoolFreeObject* Next;
	};

	CPersistentStorage<CPoolFreeObject, objectGroupSize, GeneralAllocator, GroupAllocator> createdObjects;
	CPoolFreeObject* firstFreeObject = nullptr;
};

//////////////////////////////////////////////////////////////////////////

template <class T, int objectGroupSize, class GroupAllocator, class GeneralAllocator>
CPoolRef<T, objectGroupSize, GroupAllocator, GeneralAllocator>::~CPoolRef()
{
	if( object != nullptr ) {
		pool.Release( *object );
	}
}

//////////////////////////////////////////////////////////////////////////

template <class T, int objectGroupSize, class GroupAllocator /*= CRuntimeHeap*/, class GeneralAllocator /*= CRuntimeHeap*/>
auto CObjectPool<T, objectGroupSize, GroupAllocator, GeneralAllocator>::GetOrCreate()
{
	if( firstFreeObject != nullptr ) {
		T& result = *firstFreeObject;
		firstFreeObject = firstFreeObject->Next;
		return TPoolRef( *this, result );
	} else {
		return TPoolRef( *this, createdObjects.Add() );
	}
}

template <class T, int objectGroupSize, class GroupAllocator /*= CRuntimeHeap*/, class GeneralAllocator /*= CRuntimeHeap*/>
void CObjectPool<T, objectGroupSize, GroupAllocator, GeneralAllocator>::Release( T& object )
{
	CPoolFreeObject& poolObject = static_cast<CPoolFreeObject&>( object );
	poolObject.Next = firstFreeObject;
	firstFreeObject = &poolObject;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

