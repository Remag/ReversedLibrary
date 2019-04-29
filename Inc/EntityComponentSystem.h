#pragma once
#include <PtrOwner.h>
#include <Array.h>
#include <Systems.h>

namespace Relib {

class CEntity;
class CConstEntity;
class CEntityGroup;
class CEntityRef;
class CEntityConstRef;
class CEntityGroupRange;
class CEntityGroupConstRange;
class CEntityContainer;
class CEntityInitializer;
class CComponentGroup;
template <class System>
class CSystemOwner;
class CEntityInitializationData;
//////////////////////////////////////////////////////////////////////////

// General controller of the entity-component-system data.
// Provides routines for adding/removing entities and running systems.
// Systems are separated into update systems and draw systems. Draw systems are guaranteed to never add, remove or modify entities.
class REAPI CEntityComponentSystem {
public:
	// Create an ECS. A valid entity container must be provided before any entity operations take place.
	explicit CEntityComponentSystem( CEntityContainer* container );
	~CEntityComponentSystem();

	CEntityContainer* GetEntityContainer()
		{ return entities; }
	void SetEntityContainer( CEntityContainer* newValue )
			{ entities = newValue; }

	// Delete all entities.
	void ClearEntities();

	// Create an entity. The returned object can be used to initialize component data.
	CEntity CreateEntity( const CComponentGroup& componentGroup );
	// Entity initialization process using the initializer class.
	CEntityInitializer StartEntityInitialization( CEntityInitializationData& initData );
	void AbortEntityInitialization( CEntityInitializer&& initializer );
	CEntity FinishEntityInitialization( CEntityInitializer&& initializer );

	void DestroyEntity( CEntity entity );

	// Add a new system for entity behavior control.
	// A system owner is returned that can be used to remove the system later.
	template <class System>
	CSystemOwner<System> AddSystem( CPtrOwner<System> newSystem );

	// Get all the entities that have the specified group.
	CEntityGroupRange Entities( const CComponentGroup& group );
	CEntityGroupConstRange Entities( const CComponentGroup& group ) const;

	void RunUpdateSystems( ISystemContext& context );
	void RunDrawSystems( const ISystemContext& context ) const;

	// System owners need access for system removal.
	template <class System>
	friend class CSystemOwner;

private:
	struct CUpdateSystemInfo {
		CPtrOwner<IBaseSystem> System;
		int Priority;
		const CComponentGroup* TargetGroup;
	};
	struct CDrawSystemInfo {
		CPtrOwner<IBaseSystem> System;
		int Priority;
		const CComponentGroup* TargetGroup;
	};

	CArray<CUpdateSystemInfo> writeSystems;
	CArray<CDrawSystemInfo> readSystems;

	// Container for entity data. Owned elsewhere.
	CEntityContainer* entities = nullptr;
	
	template <class System>
	void removeSystem( const System* ptr );

	void doAddSystem( IUpdateSystem* systemPtr, CPtrOwner<IUpdateSystem> newSystem );
	void doAddSystem( IDrawSystem* systemPtr, CPtrOwner<IDrawSystem> newSystem );
	void doAddSystem( IWriteSystem* systemPtr, CPtrOwner<IWriteSystem> newSystem );
	void doAddSystem( IReadSystem* systemPtr, CPtrOwner<IReadSystem> newSystem );

	void doRemoveSystem( const IWriteSystem* ptr );
	void doRemoveSystem( const IUpdateSystem* ptr );
	void doRemoveSystem( const IReadSystem* ptr );
	void doRemoveSystem( const IDrawSystem* ptr );

	void runWriteSystem( CUpdateSystemInfo& target, ISystemContext& context );
	void runUpdateSystem( CUpdateSystemInfo& target, ISystemContext& context );

	void runReadSystem( const CDrawSystemInfo& target, const ISystemContext& context ) const;
	void runDrawSystem( const CDrawSystemInfo& target, const ISystemContext& context ) const;

	// Copying and moving is prohibited.
	CEntityComponentSystem( CEntityComponentSystem& ) = delete;
	void operator=( CEntityComponentSystem& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class System>
CSystemOwner<System> CEntityComponentSystem::AddSystem( CPtrOwner<System> newSystem )
{
	const auto ptr = newSystem.Ptr();
	doAddSystem( ptr, move( newSystem ) );
	return CSystemOwner<System>( *ptr, *this );
}

template <class System>
void CEntityComponentSystem::removeSystem( const System* ptr )
{
	doRemoveSystem( ptr );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

