#include <EntityGroup.h>
#include <ComponentGroup.h>
#include <EntityRange.h>
#include <GrowStrategy.h>
#include <EntityInitializer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CEntityGroup::CEntityGroup( CComponentGroup _componentGroup ) :
	componentGroup( move( _componentGroup ) )
{
	const auto components = componentGroup.GetComponents();
	const auto componentCount = components.Size();
	if( componentCount == 0 ) {
		return;
	}
	int trivialComponentCount = 0;
	for( auto component : components ) {
		if( component->GetDestroyFunction() == nullptr ) {
			trivialComponentCount++;
		}
	}
	const auto destructibleComponentCount = componentCount - trivialComponentCount;

	// Create entity data.
	auto minComponentId = INT_MAX;
	auto maxComponentId = 0;
	entityDataOwner.ResetBuffer( trivialComponentCount );
	destructibleEntityDataOwner.ResetBuffer( destructibleComponentCount );
	for( auto component : components ) {
		const auto id = component->GetComponentId();
		minComponentId = min( minComponentId, id );
		maxComponentId = max( maxComponentId, id );

		const auto componentSize = component->GetSize();
		if( component->GetDestroyFunction() == nullptr ) {
			entityDataOwner.Add( CMemoryOwner<>{}, componentSize );
		} else {
			destructibleEntityDataOwner.Add( componentSize, component->GetConstructFunction(), component->GetDestroyFunction(), 
				component->GetMoveConstructFunction(), component->GetMoveAssignFunction() );
		}
	}

	// Fill component index.
	componentIdOffset = minComponentId;
	entityDataIndex.ResetSize( maxComponentId + 1 - minComponentId );
	int destructiblePos = 0;
	int trivialPos = 0;
	for( int i = 0; i < componentCount; i++ ) {
		const auto id = components[i]->GetComponentId() - minComponentId;
		if( components[i]->GetDestroyFunction() == nullptr ) {
			entityDataIndex[id] = &entityDataOwner[trivialPos];
			trivialPos++;
		} else {
			entityDataIndex[id] = &destructibleEntityDataOwner[destructiblePos];
			destructiblePos++;
		}
	}
}

void CEntityGroup::Empty()
{
	const auto elemCount = entityDataPtrs.Size();
	for( auto& data : destructibleEntityDataOwner ) {
		const auto byteSize = data.ElemSize * elemCount;
		data.DestroyPtr( data.Data.Ptr(), byteSize );
	}

	entityDataPtrs.Empty();
}

CEntityRange CEntityGroup::Entities()
{
	return CEntityRange{ *this, 0, entityDataPtrs.Size() };
}

CEntityConstRange CEntityGroup::Entities() const
{
	return CEntityConstRange{ *this, 0, entityDataPtrs.Size() };
}

int CEntityGroup::AddEntity( CFullEntityData* newData )
{
	const auto newIndex = entityDataPtrs.Size();
	if( entityDataPtrs.Capacity() == newIndex ) {
		growEntityDataSize();
	}
	entityDataPtrs.AddWithinCapacity( newData );
	return newIndex;
}

int CEntityGroup::InitializeEntity( CFullEntityData* newData, RelibInternal::CFilledEntityData&& filledData )
{
	const auto resultIndex = AddEntity( newData );
	const auto trivialCompCount = filledData.GetTrivialComponentCount();
	for( int i = 0; i < trivialCompCount; i++ ) {
		const auto componentData = filledData.GetTrivialData( i );
		const auto id = componentData.Id - componentIdOffset;
		const auto size = entityDataIndex[id]->ElemSize;
		const auto data = reinterpret_cast<BYTE*>( entityDataIndex[id]->Data.Ptr() );
		::memcpy( data + resultIndex * size, componentData.Data, size );
	}

	const auto destructibleCompCount = filledData.GetDestructibleComponentCount();
	for( int i = 0; i < destructibleCompCount; i++ ) {
		auto componentData = filledData.GetDestructibleData( i );
		const auto id = componentData.Id - componentIdOffset;
		auto destructibleData = static_cast<CDestructibleComponentData*>( entityDataIndex[id] );
		const auto size = destructibleData->ElemSize;
		const auto data = reinterpret_cast<BYTE*>( destructibleData->Data.Ptr() );
		const auto destData = data + resultIndex * size;
		destructibleData->MoveAssignPtr( componentData.Data, destData );
	}

	return resultIndex;
}

CFullEntityData* CEntityGroup::MoveEntity( int srcIndex, int destIndex )
{
	assert( srcIndex != destIndex );
	const auto srcData = entityDataPtrs[srcIndex];
	moveEntityData( srcIndex, destIndex );
	entityDataPtrs[destIndex] = srcData;
	return srcData;
}

void CEntityGroup::DeleteLastEntity()
{
	const auto lastDataPos = entityDataPtrs.Size() - 1;
	for( auto& data : destructibleEntityDataOwner ) {
		const auto byteSize = data.ElemSize;
		const auto bytePtr = static_cast<BYTE*>( data.Data.Ptr() );
		data.DestroyPtr( bytePtr + lastDataPos * byteSize, byteSize );
	}

	entityDataPtrs.DeleteLast();
}

void CEntityGroup::growEntityDataSize()
{
	const auto oldSize = entityDataPtrs.Capacity();
	const auto newMinSize = oldSize + 1;
	const auto newSize = CDefaultGrowStrategy<8>::GrowValue( oldSize, newMinSize );
	entityDataPtrs.ReserveBuffer( newSize );

	for( auto& data : entityDataOwner ) {
		const auto buffer = data.Data.Ptr();
		const auto oldByteSize = oldSize * data.ElemSize;
		const auto newByteSize = newSize * data.ElemSize;
		CMemoryOwner<CRuntimeHeap> newBuffer{ CRuntimeHeap::Allocate( newByteSize ) };
		const auto newBufferPtr = static_cast<BYTE*>( newBuffer.Ptr() );
		::memcpy( newBufferPtr, buffer, oldByteSize );
		::memset( newBufferPtr + oldByteSize, 0, newByteSize - oldByteSize );
		data.Data = move( newBuffer );
	}

	for( auto& data : destructibleEntityDataOwner ) {
		const auto buffer = data.Data.Ptr();
		const auto oldByteSize = oldSize * data.ElemSize;
		const auto newByteSize = newSize * data.ElemSize;
		CMemoryOwner<CRuntimeHeap> newBuffer{ CRuntimeHeap::Allocate( newByteSize ) };
		const auto newBufferPtr = static_cast<BYTE*>( newBuffer.Ptr() );
		data.MoveConstructPtr( buffer, newBufferPtr, oldByteSize );
		data.DestroyPtr( buffer, oldByteSize );
		data.ConstructPtr( newBufferPtr + oldByteSize, newByteSize - oldByteSize );
		data.Data = move( newBuffer );
	}
}

void CEntityGroup::moveEntityData( int srcIndex, int destIndex )
{
	for( auto& data : entityDataOwner ) {
		const auto buffer = static_cast<BYTE*>( data.Data.Ptr() );
		const auto byteSize = data.ElemSize;
		::memcpy( buffer + destIndex * byteSize, buffer + srcIndex * byteSize, byteSize );
	}

	for( auto& data : destructibleEntityDataOwner ) {
		const auto buffer = static_cast<BYTE*>( data.Data.Ptr() );
		const auto byteSize = data.ElemSize;
		data.MoveAssignPtr( buffer + srcIndex * byteSize, buffer + destIndex * byteSize );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
