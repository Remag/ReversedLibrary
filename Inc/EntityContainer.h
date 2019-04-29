#pragma once
#include <Array.h>
#include <ArrayBuffer.h>
#include <DynamicBitSet.h>
#include <PersistentStorage.h>
#include <PtrOwner.h>

namespace Relib {

class CEntity;
class CConstEntity;
class CEntityInitializer;
struct CFullEntityData;
//////////////////////////////////////////////////////////////////////////

// Entity and entity group data storage.
class REAPI CEntityContainer {
public:
	CEntityContainer();
	~CEntityContainer();

	void Empty();

	CArrayBuffer<CPtrOwner<CEntityGroup>> EntityGroups()
		{ return entityGroups; }
	CArrayView<CPtrOwner<CEntityGroup>> EntityGroups() const
		{ return entityGroups; }

	CEntityGroup& GetEntityGroup( int groupIndex );
	const CEntityGroup& GetEntityGroup( int groupIndex ) const;
	// Find an entity group that has all the components from the given component group.
	int MatchNextEntityGroup( int startIndex, const CComponentGroup& componentGroup ) const;

	// Create an entity with components from the given group.
	CFullEntityData& CreateEntity( const CComponentGroup& componentGroup );

	// Create an entity that is not associated with a group. The only valid action for this entity is taking and storing its reference for future use.
	CFullEntityData& CreateEmptyEntity();
	// Destroy an entity without associating it with an entity group.
	void ReturnEmptyEntity( CEntityInitializer&& initializer );
	// Associate an empty entity with a group using the initializer data for its components.
	CFullEntityData& FillEntity( CEntityInitializer&& initializer );

	void DestroyEntity( CEntity entity );

private:
	CPersistentStorage<CFullEntityData, 512> entityList;
	CArray<CFullEntityData*> freeDataList;
	CArray<CPtrOwner<CEntityGroup>> entityGroups;

	void returnEmptyEntity( CFullEntityData& entityData );
	bool doesRightContainLeft( const CDynamicBitSet<>& left, const CDynamicBitSet<>& right ) const;
	bool isComponentSetEqual( const CDynamicBitSet<>& left, const CDynamicBitSet<>& right ) const;
	CFullEntityData* createEntityData();
	CEntityGroup& getOrCreateEntityGroup( const CComponentGroup& componentGroup );

	// Copying is prohibited.
	CEntityContainer( CEntityContainer& ) = delete;
	void operator=( CEntityContainer& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
