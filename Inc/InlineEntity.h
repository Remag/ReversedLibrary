#pragma once
#include <Redefs.h>
#include <ComponentEnumerators.h>
#include <InlineComponent.h>
#include <GrowStrategy.h>
#include <RawBuffer.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Function that calls the destroy method of the component storage.
template <class T>
void CallDestructorCleanupFunction( CStaticArray<BYTE>& storage, int offset )
{
	T* objPtr = reinterpret_cast<T*>( &storage[offset] );
	objPtr->~T();
}

// Function that moves a component from one storage to another.
template <class T>
void MoveConstructFunction( CStaticArray<BYTE>& src, CStaticArray<BYTE>& dest, int offset )
{
	T* srcPtr = reinterpret_cast<T*>( &src[offset] );
	::new( &dest[offset] ) T( move( *srcPtr ) );
}

//////////////////////////////////////////////////////////////////////////

// Base inline entity class that provides component id based access to its storage.
// No component class checks are performed.
template <class StoragePtr>
class CBaseInlineEntity {
public:
	CBaseInlineEntity() : storage( nullptr ) {}
	explicit CBaseInlineEntity( StoragePtr _storage ) : storage( move( _storage ) ) {}

	bool IsNull() const
		{ return storage == nullptr; }

	// Iterate through all present component ids.
	CComponentIdEnumerator ValidComponents() const;

	void ReserveBuffer( int newValue );

protected:
	StoragePtr& getStorage()
		{ return storage; }
	const StoragePtr& getStorage() const
		{ return storage; }

	template <class Component>
	bool hasValue( const Component& component ) const;
	// Get component values.
	template <class T, class Component>
	T getValue( const Component& component ) const;
	// Get the component value or the default value if it doesn't exists.
	template <class T, class Component>
	T getValue( const Component& component, T defaultValue ) const;
	// Get named component buffers. The buffers may be invalidated after a modifying operation on the entity.

	typename Types::RawBufferType<StoragePtr>::Result getRawValue( const CNamedInlineComponent& component );
	template <class Component>
	CConstRawBuffer getRawValue( const CNamedInlineComponent& component ) const
		{ return const_cast<CBaseInlineEntity<StoragePtr>*>( this )->getRawValue( component ); }

	// Create new component value. It must not be present in storage.
	// Returned reference is invalidated as soon as another value is created in the entity.
	template <class T, class Component, class... ConstructorArgs>
	T& createNewValue( const Component& component, ConstructorArgs&&... defaultArgs );
	// Set the component value. Value is created if it doesn't exist.
	template <class T, class Component, class... ConstructorArgs>
	void setValue( const Component& component, ConstructorArgs&&... args );
	// Write the component value directly from a buffer.
	void writeRawValue( const CNamedInlineComponent& component, CConstRawBuffer buffer );

	// Get a reference to the underlying entity. The reference may be invalidated after adding other components.
	template <class T, class Component>
	decltype( auto ) getMappedValue( const Component& component );
	template <class T, class Component>
	const T& getMappedValue( const Component& component ) const;

private:
	StoragePtr storage;

	static const int collisionIndexMark = SHRT_MAX;
	static const int overflowGroupSize = 4;

	void reallocateBuffer( int newSize );

	bool hasComponent( int componentId ) const;
	int getComponentOffset( int componentId ) const;

	CEntityIndexData& getIndexData( int componentId );
	CEntityIndexData getIndexData( int componentId ) const;
	int findOverflowPos( int startPos, int componentId ) const;

	template <class T>
	int createComponentOffset( int componentId );
	int getOrCreateComponentOffset( int componentId, int dataSize, int dataAlign );

	template <class T>
	void createNewComponentGroup( CEntityIndexData& linkIndexData, int componentId );
	void createNewComponentGroup( CEntityIndexData& linkIndexData, int componentId, int dataSize, int dataAlign );
	CEntityIndexData& initializeOverflowZone( CEntityIndexData& linkIndexData );

	template <class T>
	CEntityIndexData createComponentData( int componentId );
	CEntityIndexData createSimpleComponentData( int componentId, int dataSize, int dataAlign );
	void growComponentData( int minNewSize );
	int findAlignedOffset( int alignment ) const;

	template <class T>
	T& getMappedValueFromOffset( int offset );
	template <class T, class... ConstructorArgs>
	void createMappedValueFromOffset( int offset, ConstructorArgs&&... args );

	CConstRawBuffer createRawBuffer( const void* ptr, int size ) const;
	CRawBuffer createRawBuffer( void* ptr, int size );
};

//////////////////////////////////////////////////////////////////////////

template <class StoragePtr>
CComponentIdEnumerator CBaseInlineEntity<StoragePtr>::ValidComponents() const
{
	return CComponentIdEnumerator( storage->OffsetIndex, storage->OffsetOverflowData );
}

template <class StoragePtr>
void CBaseInlineEntity<StoragePtr>::ReserveBuffer( int newValue )
{
	if( storage->ComponentData.Size() >= newValue ) {
		return;
	}

	reallocateBuffer( newValue );
}

template <class StoragePtr>
void CBaseInlineEntity<StoragePtr>::reallocateBuffer( int newSize )
{
	CStaticArray<BYTE> newData;
	newData.ResetSizeNoInitialize( newSize );
	const auto oldSize = storage->CurrentOffset;
	if( oldSize > 0 ) {
		::memcpy( newData.Ptr(), storage->ComponentData.Ptr(), oldSize );

		for( auto function : storage->CleanupFunctions ) {
			function.MoveFunction( storage->ComponentData, newData, function.Offset );
		}
	}

	storage->ComponentData = move( newData );
}

template <class StoragePtr>
template <class Component>
bool CBaseInlineEntity<StoragePtr>::hasValue( const Component& component ) const
{
	return hasComponent( component.GetId() );
}

template <class StoragePtr>
bool CBaseInlineEntity<StoragePtr>::hasComponent( int componentId ) const
{
	return getComponentOffset( componentId ) != NotFound;
}

template <class StoragePtr>
int CBaseInlineEntity<StoragePtr>::getComponentOffset( int componentId ) const
{
	assert( componentId < collisionIndexMark );
	const auto indexData = getIndexData( componentId );
	if( indexData.ComponentId == componentId ) {
		return indexData.OffsetData;
	} else if( indexData.ComponentId == collisionIndexMark ) {
		const auto overflowPos = findOverflowPos( indexData.OffsetData, componentId );
		const CArrayView<CEntityIndexData> overflowZone = storage->OffsetOverflowData;
		return overflowZone[overflowPos].ComponentId == componentId ? overflowZone[overflowPos].OffsetData : NotFound;
	} else {
		return NotFound;
	}
}

template <class StoragePtr>
CEntityIndexData& CBaseInlineEntity<StoragePtr>::getIndexData( int componentId )
{
	const auto hashTableSize = storage->OffsetIndex.Size();
	const auto hash = componentId & ( hashTableSize - 1 );
	return storage->OffsetIndex[hash];
}

// Method needs to be duplicated because OffsetIndex contains a const reference for some instances of StoragePtr.
template <class StoragePtr>
CEntityIndexData CBaseInlineEntity<StoragePtr>::getIndexData( int componentId ) const
{
	const auto hashTableSize = storage->OffsetIndex.Size();
	const auto hash = componentId & ( hashTableSize - 1 );
	return storage->OffsetIndex[hash];
}

template <class StoragePtr>
int CBaseInlineEntity<StoragePtr>::findOverflowPos( int startPos, int componentId ) const
{
	const CArrayView<CEntityIndexData> overflowZone = storage->OffsetOverflowData;
	const auto overflowGroupEnd = startPos + overflowGroupSize - 1;
	for( int i = startPos; i < overflowGroupEnd; i++ ) {
		const auto overflowId = overflowZone[i].ComponentId;
		if( overflowId == componentId || overflowId == NotFound ) {
			return i;
		}
	}

	const auto lastId = overflowZone[overflowGroupEnd].ComponentId;
	if( lastId == collisionIndexMark ) {
		return findOverflowPos( overflowZone[overflowGroupEnd].OffsetData, componentId );
	} else {
		return overflowGroupEnd;
	}
}

template <class StoragePtr>
template <class T, class Component>
T CBaseInlineEntity<StoragePtr>::getValue( const Component& component ) const
{
	const int componentId = component.GetId();
	const auto offset = getComponentOffset( componentId );
	assert( offset >= 0 && offset < storage->CurrentOffset );
	return CConstRawBuffer( &storage->ComponentData[offset], component.GetSize() ).As<T>();
}

template <class StoragePtr>
template <class T, class Component>
T CBaseInlineEntity<StoragePtr>::getValue( const Component& component, T defaultValue ) const
{
	const int componentId = component.GetId();
	const auto offset = getComponentOffset( componentId );
	if( offset == NotFound ) {
		return defaultValue;
	}
	return CConstRawBuffer( &storage->ComponentData[offset], component.GetSize() ).As<T>();
}

template <class StoragePtr>
typename Types::RawBufferType<StoragePtr>::Result CBaseInlineEntity<StoragePtr>::getRawValue( const CNamedInlineComponent& component )
{
	const int componentId = component.GetId();
	const auto offset = getComponentOffset( componentId );
	assert( offset >= 0 && offset < storage->CurrentOffset );
	return createRawBuffer( &storage->ComponentData[offset], component.GetSize() );
}

template <class StoragePtr>
template <class T, class Component, class... ConstructorArgs>
T& CBaseInlineEntity<StoragePtr>::createNewValue( const Component& component, ConstructorArgs&&... defaultArgs )
{
	const int componentId = component.GetId();
	const auto offset = createComponentOffset<T>( componentId );
	return *::new( &storage->ComponentData[offset] ) T( forward<ConstructorArgs>( defaultArgs )... );
}

template <class StoragePtr>
template <class T>
int CBaseInlineEntity<StoragePtr>::createComponentOffset( int componentId )
{
	assert( componentId < collisionIndexMark );
	auto& indexData = getIndexData( componentId );
	assert( indexData.ComponentId != componentId );
	if( indexData.ComponentId == NotFound ) {
		indexData = createComponentData<T>( componentId );
		return indexData.OffsetData;
	} else if( indexData.ComponentId != collisionIndexMark ) {
		// Encountered an overflow. Create a new overflow group and add its information to the index.
		createNewComponentGroup<T>( indexData, componentId );
		return indexData.OffsetData;
	} else {
		// An overflow is already present, search overflow groups.
		const auto overflowPos = findOverflowPos( indexData.OffsetData, componentId );
		auto& overflowZone = storage->OffsetOverflowData;
		auto& overflowData = overflowZone[overflowPos];
		if( overflowData.ComponentId == componentId ) {
			return overflowData.OffsetData;
		} else if( overflowData.ComponentId == NotFound ) {
			// Use an existing empty group index.
			overflowData = createComponentData<T>( componentId );
			return overflowData.OffsetData;
		} else {
			// Create a new overflow group and link it to the existing ones.
			createNewComponentGroup<T>( overflowData, componentId );
			return overflowData.OffsetData;
		}
	}
}

template <class StoragePtr>
int CBaseInlineEntity<StoragePtr>::getOrCreateComponentOffset( int componentId, int dataSize, int dataAlign )
{
	assert( componentId < collisionIndexMark );
	auto& indexData = getIndexData( componentId );
	if( indexData.ComponentId == componentId ) {
		return indexData.OffsetData;
	} else if( indexData.ComponentId == NotFound ) {
		const int alignedOffset = findAlignedOffset( dataAlign );
		indexData = createSimpleComponentData( componentId, dataSize, alignedOffset );
		return indexData.OffsetData;
	} else if( indexData.ComponentId != collisionIndexMark ) {
		// Encountered an overflow. Create a new overflow group and add its information to the index.
		createNewComponentGroup( indexData, componentId, dataSize, dataAlign );
		return indexData.OffsetData;
	} else {
		// An overflow is already present, search overflow groups.
		const auto overflowPos = findOverflowPos( indexData.OffsetData, componentId );
		auto& overflowZone = storage->OffsetOverflowData;
		auto& overflowData = overflowZone[overflowPos];
		if( overflowData.ComponentId == componentId ) {
			return overflowData.OffsetData;
		} else if( overflowData.ComponentId == NotFound ) {
			// Use an existing empty group index.
			const int alignedOffset = findAlignedOffset( dataAlign );
			overflowData = createSimpleComponentData( componentId, dataSize, alignedOffset );
			return overflowData.OffsetData;
		} else {
			// Create a new overflow group and link it to an existing ones.
			createNewComponentGroup( overflowData, componentId, dataSize, dataAlign );
			return overflowData.OffsetData;
		}
	}
}

template <class StoragePtr>
template <class T>
void CBaseInlineEntity<StoragePtr>::createNewComponentGroup( CEntityIndexData& linkIndexData, int componentId )
{
	auto& newSlot = initializeOverflowZone( linkIndexData );
	newSlot = createComponentData<T>( componentId );
}

template <class StoragePtr>
void CBaseInlineEntity<StoragePtr>::createNewComponentGroup( CEntityIndexData& linkIndexData, int componentId, int dataSize, int dataAlign )
{
	const int alignedOffset = findAlignedOffset( dataAlign );
	auto& newSlot = initializeOverflowZone( linkIndexData );
	newSlot = createSimpleComponentData( componentId, dataSize, alignedOffset );
}

template <class StoragePtr>
CEntityIndexData& CBaseInlineEntity<StoragePtr>::initializeOverflowZone( CEntityIndexData& linkIndexData )
{
	auto& overflowZone = storage->OffsetOverflowData;
	const auto newGroupPos = overflowZone.Size();
	overflowZone.IncreaseSizeNoInitialize( newGroupPos + overflowGroupSize );
	overflowZone[newGroupPos] = linkIndexData;
	for( int i = newGroupPos + 2; i < newGroupPos + overflowGroupSize; i++ ) {
		overflowZone[i] = CEntityIndexData{ NotFound, 0 };
	}
	linkIndexData.ComponentId = collisionIndexMark;
	linkIndexData.OffsetData = numeric_cast<unsigned short>( newGroupPos );
	return overflowZone[newGroupPos + 1];
}

template <class StoragePtr>
template <class T>
CEntityIndexData CBaseInlineEntity<StoragePtr>::createComponentData( int componentId )
{
	const int alignedOffset = findAlignedOffset( alignof( T ) );
	const auto result = createSimpleComponentData( componentId, sizeof( T ), alignedOffset );
	if( !Types::HasTrivialDestructor<T>::Result || !Types::HasTrivialMoveConstructor<T>::Result ) {
		storage->CleanupFunctions.Add( CStorageCleanupData( alignedOffset, RelibInternal::CallDestructorCleanupFunction<T>, RelibInternal::MoveConstructFunction<T> ) );
	}
	return result;
}

template <class StoragePtr>
CEntityIndexData CBaseInlineEntity<StoragePtr>::createSimpleComponentData( int componentId, int dataSize, int alignedOffset )
{
	const int newOffset = alignedOffset + dataSize;
	if( storage->ComponentData.Size() < newOffset ) {
		growComponentData( newOffset );
	}
	storage->CurrentOffset = newOffset;

	return CEntityIndexData{ numeric_cast<short>( componentId ), numeric_cast<unsigned short>( alignedOffset ) };
}

template <class StoragePtr>
void CBaseInlineEntity<StoragePtr>::growComponentData( int minNewSize )
{
	const auto newSize = CDefaultGrowStrategy<8>::GrowValue( storage->ComponentData.Size(), minNewSize );
	reallocateBuffer( newSize );
}

template <class StoragePtr>
int CBaseInlineEntity<StoragePtr>::findAlignedOffset( int alignment ) const
{
	return CeilTo( storage->CurrentOffset, alignment );
}

template <class StoragePtr>
CConstRawBuffer CBaseInlineEntity<StoragePtr>::createRawBuffer( const void* ptr, int size ) const
{
	return CConstRawBuffer{ ptr, size };
}

template <class StoragePtr>
CRawBuffer CBaseInlineEntity<StoragePtr>::createRawBuffer( void* ptr, int size )
{
	return CRawBuffer{ ptr, size };
}

template <class StoragePtr>
template <class T, class Component, class... ConstructorArgs>
void CBaseInlineEntity<StoragePtr>::setValue( const Component& component, ConstructorArgs&&... args )
{
	const int componentId = component.GetId();
	assert( componentId < collisionIndexMark );
	auto& indexData = getIndexData( componentId );
	if( indexData.ComponentId == componentId ) {
		getMappedValueFromOffset<T>( indexData.OffsetData ) = T( forward<ConstructorArgs>( args )... );
	} else if( indexData.ComponentId == NotFound ) {
		indexData = createComponentData<T>( componentId );
		createMappedValueFromOffset<T>( indexData.OffsetData, forward<ConstructorArgs>( args )... );
	} else if( indexData.ComponentId != collisionIndexMark ) {
		// Encountered an overflow. Create a new overflow group and add its information to the index.
		createNewComponentGroup<T>( indexData, componentId );
		createMappedValueFromOffset<T>( indexData.OffsetData, forward<ConstructorArgs>( args )... );
	} else {
		// An overflow is already present, search overflow groups.
		const auto overflowPos = findOverflowPos( indexData.OffsetData, componentId );
		auto& overflowZone = storage->OffsetOverflowData;
		auto& overflowData = overflowZone[overflowPos];
		if( overflowData.ComponentId == componentId ) {
			getMappedValueFromOffset<T>( overflowData.OffsetData ) = T( forward<ConstructorArgs>( args )... );
		} else if( overflowData.ComponentId == NotFound ) {
			// Use an existing empty group index.
			overflowData = createComponentData<T>( componentId );
			createMappedValueFromOffset<T>( overflowData.OffsetData, forward<ConstructorArgs>( args )... );
		} else {
			// Create a new overflow group and link it to the existing ones.
			createNewComponentGroup<T>( overflowData, componentId );
			createMappedValueFromOffset<T>( overflowData.OffsetData, forward<ConstructorArgs>( args )... );
		}
	}
}

template <class StoragePtr>
template <class T, class... ConstructorArgs>
void CBaseInlineEntity<StoragePtr>::createMappedValueFromOffset( int offset, ConstructorArgs&&... args )
{
	assert( offset >= 0 && offset < storage->CurrentOffset );
	::new( &storage->ComponentData[offset] ) T( forward<ConstructorArgs>( args )... );
}

template <class StoragePtr>
template <class T>
T& CBaseInlineEntity<StoragePtr>::getMappedValueFromOffset( int offset )
{
	assert( offset >= 0 && offset < storage->CurrentOffset );
	return createRawBuffer( &storage->ComponentData[offset], sizeof( T ) ).As<T>();
}

template <class StoragePtr>
void CBaseInlineEntity<StoragePtr>::writeRawValue( const CNamedInlineComponent& component, CConstRawBuffer buffer )
{
	const auto componentId = component.GetId();
	assert( componentId < collisionIndexMark );
	assert( component.GetType() != NCT_CustomComplex );
	const auto offset = getOrCreateComponentOffset( componentId, component.GetSize(), component.GetAlignment() );
	memcpy( &storage->ComponentData[offset], buffer.Ptr(), buffer.Size() );
}

template <class StoragePtr>
template <class T, class Component>
decltype( auto ) CBaseInlineEntity<StoragePtr>::getMappedValue( const Component& component )
{
	const int componentId = component.GetId();
	const auto offset = getComponentOffset( componentId );
	assert( offset >= 0 && offset < storage->CurrentOffset );
	return createRawBuffer( &storage->ComponentData[offset], component.GetSize() ).As<T>();
}

template <class StoragePtr>
template <class T, class Component>
const T& CBaseInlineEntity<StoragePtr>::getMappedValue( const Component& component ) const
{
	const int componentId = component.GetId();
	const auto offset = getComponentOffset( componentId );
	assert( offset >= 0 && offset < storage->CurrentOffset );
	return createRawBuffer( &storage->ComponentData[offset], component.GetSize() ).As<T>();
}

//////////////////////////////////////////////////////////////////////////

// An inline entity with no specified component class. Performs runtime assertions for the class id matching.
template <class StoragePtr>
class CDynamicEntity : public CBaseInlineEntity<StoragePtr> {
public:
	explicit CDynamicEntity( int _classId ) : classId( _classId ) {}
	CDynamicEntity( StoragePtr storage, int _classId ) : CBaseInlineEntity<StoragePtr>( move( storage ) ), classId( _classId ) {}
	
	operator CDynamicEntity<CInlineEntityStorage*>()
		{ return CDynamicEntity<CInlineEntityStorage*>( this->getStorage(), classId ); }
	operator CDynamicEntity<const CInlineEntityStorage*>() const
		{ return CDynamicEntity<const CInlineEntityStorage*>( this->getStorage(), classId ); }

	template <class Component>
	bool HasValue( const Component& component ) const
		{ checkClassMatch( component ); return hasValue( component ); }
	// Get component values.
	template <class T, class Component>
	T GetValue( const Component& component ) const
		{ checkClassMatch( component ); return this->getValue<T>( component ); }

	// Get named component buffers. The buffers may be invalidated after a modifying operation on the entity.
	typename Types::RawBufferType<StoragePtr>::Result GetValue( const CNamedInlineComponent& component )
		{ checkClassMatch( component ); return this->getRawValue( component ); }
	CConstRawBuffer GetValue( const CNamedInlineComponent& component ) const
		{ checkClassMatch( component ); return this->getRawValue( component ); }

	// Get the component value or the default value if it doesn't exists.
	template <class T, class Component>
	T GetValue( const Component& component, T defaultValue ) const
		{ checkClassMatch( component ); return this->getValue( component, move( defaultValue ) ); }

	// Create new component value. It must not be present in storage.
	// Returned reference is invalidated as soon as another value is created in the entity.
	template <class T, class Component, class... ConstructorArgs>
	T& CreateNewValue( const Component& component, ConstructorArgs&&... defaultArgs )
		{ checkClassMatch( component ); return this->createNewValue<T>( component, forward<ConstructorArgs>( defaultArgs )... ); }

	// Set the component value. Value is created if it doesn't exist.
	template <class T, class Component, class... ConstructorArgs>
	void SetValue( const Component& component, ConstructorArgs&&... args )
		{ checkClassMatch( component ); return this->setValue<T>( component, forward<ConstructorArgs>( args )... ); }
	// Write a value into entity directly from the given buffer. The value must be trivially movable and destructible.
	void WritePrimitiveValue( const CNamedInlineComponent& component, CConstRawBuffer buffer )
		{ checkClassMatch( component ); return this->writeRawValue( component, buffer ); }

	// Get a reference to the underlying entity. The reference may be invalidated after adding other components.
	template <class T, class Component>
	decltype( auto ) GetMappedValue( const Component& component )
		{ checkClassMatch( component ); return this->getMappedValue<T>( component ); }
	template <class T, class Component>
	const T& GetMappedValue( const Component& component ) const
		{ checkClassMatch( component ); return this->getMappedValue<T>( component ); }

private:
	int classId;

	template <class Component>
	void checkClassMatch( const Component& component ) const
		{ assert( classId == component.GetClassId() ); }
};

//////////////////////////////////////////////////////////////////////////

// A entity that provides component-based access to its data.
// The storage for the entity is a dynamic array that is owned by the entity itself.
template <class ComponentClass, class StoragePtr>
class CInlineEntity : public CBaseInlineEntity<StoragePtr> {
public:
	using CBaseInlineEntity<StoragePtr>::CBaseInlineEntity;
	
	operator CInlineEntity<ComponentClass, CInlineEntityStorage*>()
		{ return CInlineEntity<ComponentClass, CInlineEntityStorage*>( this->getStorage() ); }
	operator CInlineEntity<ComponentClass, const CInlineEntityStorage*>() const
		{ return CInlineEntity<ComponentClass, const CInlineEntityStorage*>( this->getStorage() ); }
	operator CDynamicEntity<CInlineEntityStorage*>()
		{ return CDynamicEntity<CInlineEntityStorage*>( this->getStorage(), GetComponentClassId<ComponentClass>() ); }
	operator CDynamicEntity<const CInlineEntityStorage*>() const
		{ return CDynamicEntity<const CInlineEntityStorage*>( this->getStorage(), GetComponentClassId<ComponentClass>() ); }

	template <class T>
	bool HasValue( const CInlineComponent<T, ComponentClass>& component ) const
		{ return this->hasValue( component ); }
	// Get component values.
	template <class T>
	T GetValue( const CInlineComponent<T, ComponentClass>& component ) const
		{ return this->getValue<T>( component ); }

	// Get named component buffers. The buffers may be invalidated after a modifying operation on the entity.
	typename Types::RawBufferType<StoragePtr>::Result GetValue( const CNamedInlineComponent& component )
		{ return this->getRawValue( component ); }
	CConstRawBuffer GetValue( const CNamedInlineComponent& component ) const
		{ return this->getRawValue( component ); }

	// Get the component value or the default value if it doesn't exists.
	template <class T>
	T GetValue( const CInlineComponent<T, ComponentClass>& component, T defaultValue ) const
		{ return this->getValue( component, move( defaultValue ) ); }

	// Create new component value. It must not be present in storage.
	// Returned reference is invalidated as soon as another value is created in the entity.
	template <class T, class... ConstructorArgs>
	T& CreateNewValue( const CInlineComponent<T, ComponentClass>& component, ConstructorArgs&&... defaultArgs )
		{ return this->createNewValue<T>( component, forward<ConstructorArgs>( defaultArgs )... ); }
	template <class T, class... ConstructorArgs>
	T& CreateNewValue( const CNamedInlineComponent& component, ConstructorArgs&&... defaultArgs )
		{ return this->createNewValue<T>( component, forward<ConstructorArgs>( defaultArgs )... ); }

	// Set the component value. Value is created if it doesn't exist.
	template <class T, class... ConstructorArgs>
	void SetValue( const CInlineComponent<T, ComponentClass>& component, ConstructorArgs&&... args )
		{ return this->setValue<T>( component, forward<ConstructorArgs>( args )... ); }
	template <class T, class... ConstructorArgs>
	void SetValue( const CNamedInlineComponent& component, ConstructorArgs&&... args )
		{ return this->setValue<T>( component, forward<ConstructorArgs>( args )... ); }

	// Get a reference to the underlying entity. The reference may be invalidated after adding other components.
	template <class T>
	decltype( auto ) GetMappedValue( const CInlineComponent<T, ComponentClass>& component )
		{ return this->getMappedValue<T>( component ); }
	template <class T>
	const T& GetMappedValue( const CInlineComponent<T, ComponentClass>& component ) const
		{ return this->getMappedValue<T>( component ); }
};

} // namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

template <class ComponentClass>
using CInlineEntityOwner = RelibInternal::CInlineEntity<ComponentClass, CPtrOwner<RelibInternal::CInlineEntityStorage>>;
template <class ComponentClass>
using CInlineEntityRef = RelibInternal::CInlineEntity<ComponentClass, RelibInternal::CInlineEntityStorage*>;
template <class ComponentClass>
using CInlineEntityConstRef = RelibInternal::CInlineEntity<ComponentClass, const RelibInternal::CInlineEntityStorage*>;

using CDynamicEntityOwner = RelibInternal::CDynamicEntity<CPtrOwner<RelibInternal::CInlineEntityStorage>>;
using CDynamicEntityRef = RelibInternal::CDynamicEntity<RelibInternal::CInlineEntityStorage*>;
using CDynamicEntityConstRef = RelibInternal::CDynamicEntity<const RelibInternal::CInlineEntityStorage*>;

template <class ComponentClass>
CInlineEntityOwner<ComponentClass> CreateInlineEntity()
{
	return CInlineEntityOwner<ComponentClass>( CreateOwner<RelibInternal::CInlineEntityStorage>() );
}

inline CDynamicEntityOwner CreateDynamicEntity( const IComponentClass& componentClass )
{
	return CDynamicEntityOwner( CreateOwner<RelibInternal::CInlineEntityStorage>(), componentClass.GetClassId() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

