#pragma once
#include <Atomic.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Queue-like storage that can be used for concurrent operations.
// Represented internally by a linked list of memory blocks.
// T must be default-constructible.
// Allocator is assumed to be synchronized separately.
template <class T, int blockElemCount, class Allocator = CRuntimeHeap>
class CLockFreeStorage : public CAllocationStrategy<Allocator> {
public:
	static_assert( blockElemCount > 0, "Number of elements in a storage block must be a positive value." );

	template <class... AllocatorArgs>
	CLockFreeStorage( AllocatorArgs&&... args ) : CAllocationStrategy<Allocator>( forward<AllocatorArgs>( args )... ), firstNode( nullptr ) {}

	~CLockFreeStorage()
		{ FreeBuffer(); }

	// Destroy all the values and free associated memory. This operation is not thread-safe.
	void FreeBuffer();

	// Get the current amount of accessible elements.
	// This operation is thread-safe. The amount of elements may increase at an arbitrary point in time.
	int StorageSize() const;
	bool IsEmpty() const
		{ return firstNode.Load() == nullptr; }

	// Increase the storage size to fit the given number of elements.
	// This operation is thread-safe.
	void IncreaseSize( int newElemCount );

	// Get an element with the given index. This operation is thread-safe.
	// Access to an element must be synchronized separately.
	T& operator[]( int index );
	const T& operator[]( int index ) const;

	// Get an element with the given index or resize the storage to fit the needed amount and create the element.
	// If the storage needs to be resized, new elements are default-constructed.
	// This operation is thread-safe.
	// Access to the element must be synchronized separately.
	T& GetOrCreate( int index );

private:
	struct CStorageNode {
		// Next value in the list. Null for end nodes.
		CAtomic<CStorageNode*> Next;
	};

	CAtomic<CStorageNode*> firstNode;

	static const int alignedNodeSize = CeilTo( sizeof( CStorageNode ), alignof( T ) );

	static T* getValuesFromNode( CStorageNode* node );
	void destroyNode( CStorageNode* node );
	CStorageNode* getOrAddNewNode( CAtomic<CStorageNode*>& nodePos );
	T& getValueFromNode( int index, CStorageNode* nodePtr, int elemCountAtNode );
};

//////////////////////////////////////////////////////////////////////////

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
void CLockFreeStorage<T, blockElemCount, Allocator>::FreeBuffer()
{
	CStorageNode* currentNode = firstNode.Load();
	while( currentNode != nullptr ) {
		const auto nextNode = currentNode->Next.Load();
		destroyNode( currentNode );
		currentNode = nextNode;
	}
	firstNode.Store( nullptr );
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
void CLockFreeStorage<T, blockElemCount, Allocator>::destroyNode( CStorageNode* node )
{
	assert( node != nullptr );
	const auto values = getValuesFromNode( node );
	values;
	for( int i = 0; i < blockElemCount; i++ ) {
		values[i].~T();
	}
	CAllocationStrategy::StrategyFree( node );
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
T* CLockFreeStorage<T, blockElemCount, Allocator>::getValuesFromNode( CStorageNode* node )
{
	const auto valuesPtr = reinterpret_cast<BYTE*>( node ) + alignedNodeSize;
	assert( reinterpret_cast<int>( valuesPtr ) % alignof( T ) == 0 );
	return reinterpret_cast<T*>( valuesPtr );
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
int CLockFreeStorage<T, blockElemCount, Allocator>::StorageSize() const
{
	int elemCount = 0;
	CStorageNode* currentNode = firstNode.Load();
	while( currentNode != nullptr ) {
		elemCount += blockElemCount;
		currentNode = currentNode->Next.Load();
	}
	return elemCount;
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
void CLockFreeStorage<T, blockElemCount, Allocator>::IncreaseSize( int newElemCount )
{
	assert( newElemCount >= 0 );
	CAtomic<CStorageNode*>* lastNode = &firstNode;
	int elemCount = 0;

	while( elemCount < newElemCount ) {
		assert( lastNode != nullptr );
		lastNode = &getOrAddNewNode( *lastNode )->Next;
		elemCount += blockElemCount;
	}
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
typename CLockFreeStorage<T, blockElemCount, Allocator>::CStorageNode* CLockFreeStorage<T, blockElemCount, Allocator>::getOrAddNewNode( CAtomic<CStorageNode*>& nodePos )
{
	// Quick check if the node has been added already.
	auto node = nodePos.Load();
	if( node != nullptr ) {
		return node;
	}

	// Create a new node.
	const int totalSize = alignedNodeSize + blockElemCount * sizeof( T );
	void* newNodePtr = RELIB_STRATEGY_ALLOCATE( totalSize );
	CStorageNode* newNode = ::new( newNodePtr ) CStorageNode();
	BYTE* valuesPtr = reinterpret_cast<BYTE*>( newNodePtr ) + alignedNodeSize;
	for( int i = 0; i < blockElemCount; i++ ) {
		::new( valuesPtr ) T();
		valuesPtr += sizeof( T );
	}

	// Check if node value has been created somewhere else.
	if( !nodePos.CompareExchangeStrong( node, newNode ) ) {
		destroyNode( newNode );
		return node;
	}

	return newNode;
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
T& CLockFreeStorage<T, blockElemCount, Allocator>::operator[]( int index )
{
	assert( index >= 0 );
	int elemCount = blockElemCount;
	CStorageNode* targetNode = firstNode.Load();
	while( elemCount <= index ) {
		assert( targetNode != nullptr );
		targetNode = targetNode->Next.Load();
		elemCount += blockElemCount;
	}

	return getValueFromNode( index, targetNode, elemCount );
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
T& CLockFreeStorage<T, blockElemCount, Allocator>::getValueFromNode( int index, CStorageNode* nodePtr, int elemCountAtNode )
{
	assert( nodePtr != nullptr );
	const int valuePos = index - ( elemCountAtNode - blockElemCount );
	const auto values = getValuesFromNode( nodePtr );
	return values[valuePos];
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
const T& CLockFreeStorage<T, blockElemCount, Allocator>::operator[]( int index ) const
{
	return const_cast<CLockFreeStorage<T, blockElemCount, Allocator>*>( this )->operator[]( index );
}

template <class T, int blockElemCount, class Allocator /*= CRuntimeHeap*/>
T& CLockFreeStorage<T, blockElemCount, Allocator>::GetOrCreate( int index )
{
	assert( index >= 0 );
	CStorageNode* targetNode = getOrAddNewNode( firstNode );
	int elemCount = blockElemCount;

	while( elemCount <= index ) {
		targetNode = getOrAddNewNode( targetNode->Next );
		elemCount += blockElemCount;
	}
	return getValueFromNode( index, targetNode, elemCount );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

