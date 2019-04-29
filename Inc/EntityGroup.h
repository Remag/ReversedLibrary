#pragma once
#include <Redefs.h>
#include <EntityRef.h>
#include <Entity.h>
#include <Array.h>
#include <StaticArray.h>
#include <MemoryOwner.h>
#include <ComponentGroup.h>

namespace Relib {

class CComponentGroup;
class CEntityRange;
class CEntityConstRange;
//////////////////////////////////////////////////////////////////////////

// Structure of arrays of entity data.
class REAPI CEntityGroup {
public:
	explicit CEntityGroup( CComponentGroup componentGroup );

	const CComponentGroup& GetComponentGroup() const
		{ return componentGroup; }

	int Size() const
		{ return entityDataPtrs.Size(); }
	void Empty();

	CEntityRange Entities();
	CEntityConstRange Entities() const;

	CFullEntityData* GetEntityData( int entityIndex ) const
		{ return entityDataPtrs[entityIndex]; }

	template <class T>
	T& GetValue( const CComponent<T>& component, int entityIndex );
	template <class T>
	T* TryGetValue( const CComponent<T>& component, int entityIndex );

	int AddEntity( CFullEntityData* newData );
	int InitializeEntity( CFullEntityData* newData, RelibInternal::CFilledEntityData&& filledData );

	// Move data corresponding to a given entity. Return the moved entity data info.
	CFullEntityData* MoveEntity( int srcIndex, int destIndex );
	// Delete the last entity in the list.
	void DeleteLastEntity();

private:
	struct CComponentData {
		CMemoryOwner<> Data;
		int ElemSize = 0;
	};
	struct CDestructibleComponentData : CComponentData {
		CBaseComponent::TConstructComponentData ConstructPtr;
		CBaseComponent::TDestroyComponentData DestroyPtr;
		CBaseComponent::TMoveConstructComponentData MoveConstructPtr;
		CBaseComponent::TMoveAssignComponentData MoveAssignPtr;

		CDestructibleComponentData( int elemSize, CBaseComponent::TConstructComponentData constructPtr, CBaseComponent::TDestroyComponentData destroyPtr,
				CBaseComponent::TMoveConstructComponentData moveConstructPtr, CBaseComponent::TMoveAssignComponentData moveAssignPtr ) :
			CComponentData{ CMemoryOwner<>{}, elemSize }, ConstructPtr( constructPtr ), DestroyPtr( destroyPtr ), MoveConstructPtr( moveConstructPtr ), MoveAssignPtr( moveAssignPtr ) {}
	};

	// Corresponding component group.
	CComponentGroup componentGroup;
	// Value of the smallest component id in the group.
	// This value is subtracted from component ids before accessing the index.
	int componentIdOffset = 0;
	// Entity data indexed by component id.
	CStaticArray<CComponentData> entityDataOwner;
	CStaticArray<CDestructibleComponentData> destructibleEntityDataOwner;
	CStaticArray<CComponentData*> entityDataIndex;
	// List of entity full data information.
	CArray<CFullEntityData*> entityDataPtrs;

	void growEntityDataSize();
	void moveEntityData( int srcIndex, int destIndex );
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T& CEntityGroup::GetValue( const CComponent<T>& component, int entityIndex )
{
	const auto id = component.GetComponentId() - componentIdOffset;
	assert( entityDataIndex[id]->ElemSize == sizeof( T ) );
	const auto data = reinterpret_cast<BYTE*>( entityDataIndex[id]->Data.Ptr() );
	const auto resultData = data + entityIndex * sizeof( T );
	return *reinterpret_cast<T*>( resultData );
}

template <class T>
T* CEntityGroup::TryGetValue( const CComponent<T>& component, int entityIndex )
{
	const auto id = component.GetComponentId() - componentIdOffset;
	if( id < 0 || id >= entityDataIndex.Size() || entityDataIndex[id]->ElemSize != sizeof( T ) ) {
		return nullptr;
	}
	const auto data = reinterpret_cast<BYTE*>( entityDataIndex[id]->Data.Ptr() );
	const auto resultData = data + entityIndex * sizeof( T );
	return reinterpret_cast<T*>( resultData );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

