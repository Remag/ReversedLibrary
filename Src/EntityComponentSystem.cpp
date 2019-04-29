#include <EntityComponentSystem.h>
#include <EntityRef.h>
#include <ReSearch.h>
#include <ComponentGroup.h>
#include <EntityContainer.h>
#include <EntityGroup.h>
#include <EntityRange.h>
#include <Systems.h>
#include <SystemOwner.h>
#include <EntityGroupRange.h>
#include <EntityInitializer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CEntityComponentSystem::CEntityComponentSystem( CEntityContainer* container ) : 
	entities( container )
{
}

CEntityComponentSystem::~CEntityComponentSystem()
{
}

void CEntityComponentSystem::ClearEntities()
{
	assert( entities != nullptr );
	entities->Empty();
}

CEntity CEntityComponentSystem::CreateEntity( const CComponentGroup& componentGroup )
{
	assert( entities != nullptr );
	auto& fullData = entities->CreateEntity( componentGroup );
	return fullData.Entity;
}

CEntityInitializer CEntityComponentSystem::StartEntityInitialization( CEntityInitializationData& initData )
{
	auto& fullInfo = entities->CreateEmptyEntity();
	return CEntityInitializer( initData, fullInfo );
}

void CEntityComponentSystem::AbortEntityInitialization( CEntityInitializer&& initializer )
{
	entities->ReturnEmptyEntity( move( initializer ) );
}

CEntity CEntityComponentSystem::FinishEntityInitialization( CEntityInitializer&& initializer )
{
	auto& fullInfo = entities->FillEntity( move( initializer ) );
	return fullInfo.Entity;
}

void CEntityComponentSystem::DestroyEntity( CEntity entity )
{
	assert( entities != nullptr );
	entities->DestroyEntity( entity );
}

void CEntityComponentSystem::doAddSystem( IUpdateSystem*, CPtrOwner<IUpdateSystem> newSystem )
{
	const auto newPriority = newSystem->GetPriority();
	const auto systemPos = SearchSortedPos( writeSystems, newPriority, WeakGreaterByAction( &CUpdateSystemInfo::Priority ) );
	const auto& newGroup = newSystem->GetTargetGroup();
	writeSystems.InsertAt( systemPos, move( newSystem ), newPriority, &newGroup );
}

void CEntityComponentSystem::doAddSystem( IDrawSystem*, CPtrOwner<IDrawSystem> newSystem )
{
	const auto newPriority = newSystem->GetPriority();
	const auto systemPos = SearchSortedPos( readSystems, newPriority, WeakGreaterByAction( &CDrawSystemInfo::Priority ) );
	const auto& newGroup = newSystem->GetTargetGroup();
	readSystems.InsertAt( systemPos, move( newSystem ), newPriority, &newGroup );
}

void CEntityComponentSystem::doAddSystem( IWriteSystem*, CPtrOwner<IWriteSystem> newSystem )
{
	const auto newPriority = newSystem->GetPriority();
	const auto systemPos = SearchSortedPos( writeSystems, newPriority, WeakGreaterByAction( &CUpdateSystemInfo::Priority ) );
	writeSystems.InsertAt( systemPos, move( newSystem ), newPriority, nullptr );
}

void CEntityComponentSystem::doAddSystem( IReadSystem*, CPtrOwner<IReadSystem> newSystem )
{
	const auto newPriority = newSystem->GetPriority();
	const auto systemPos = SearchSortedPos( readSystems, newPriority, WeakGreaterByAction( &CDrawSystemInfo::Priority ) );
	readSystems.InsertAt( systemPos, move( newSystem ), newPriority, nullptr );
}

void CEntityComponentSystem::doRemoveSystem( const IWriteSystem* ptr )
{
	const auto systemPos = SearchPos( writeSystems, ptr, EqualByAction( &CUpdateSystemInfo::System ) );
	assert( systemPos != NotFound );
	writeSystems.DeleteAt( systemPos );
}

void CEntityComponentSystem::doRemoveSystem( const IUpdateSystem* ptr )
{
	const auto systemPos = SearchPos( writeSystems, ptr, EqualByAction( &CUpdateSystemInfo::System ) );
	assert( systemPos != NotFound );
	writeSystems.DeleteAt( systemPos );
}

void CEntityComponentSystem::doRemoveSystem( const IReadSystem* ptr )
{
	const auto systemPos = SearchPos( readSystems, ptr, EqualByAction( &CDrawSystemInfo::System ) );
	assert( systemPos != NotFound );
	readSystems.DeleteAt( systemPos );
}

void CEntityComponentSystem::doRemoveSystem( const IDrawSystem* ptr )
{
	const auto systemPos = SearchPos( readSystems, ptr, EqualByAction( &CDrawSystemInfo::System ) );
	assert( systemPos != NotFound );
	readSystems.DeleteAt( systemPos );
}

CEntityGroupRange CEntityComponentSystem::Entities( const CComponentGroup& group )
{
	assert( entities != nullptr );
	return CEntityGroupRange( *entities, group );
}

CEntityGroupConstRange CEntityComponentSystem::Entities( const CComponentGroup& group ) const
{
	assert( entities != nullptr );
	return CEntityGroupConstRange( *entities, group );
}

void CEntityComponentSystem::RunUpdateSystems( ISystemContext& context )
{
	for( auto& system : writeSystems ) {
		if( system.TargetGroup == nullptr ) {
			runWriteSystem( system, context );
		} else {
			runUpdateSystem( system, context );
		}
	}
}

void CEntityComponentSystem::runWriteSystem( CUpdateSystemInfo& target, ISystemContext& context )
{
	auto& system = static_cast<IWriteSystem&>( *target.System.Ptr() );
	system.RunGeneralUpdate( context );
}

void CEntityComponentSystem::runUpdateSystem( CUpdateSystemInfo& target, ISystemContext& context )
{
	auto& system = static_cast<IUpdateSystem&>( *target.System.Ptr() );
	system.RunEntityListUpdate( Entities( *target.TargetGroup ), context );
}

void CEntityComponentSystem::RunDrawSystems( const ISystemContext& context ) const
{
	for( const auto& system : readSystems ) {
		if( system.TargetGroup == nullptr ) {
			runReadSystem( system, context );
		} else {
			runDrawSystem( system, context );
		}
	}
}

void CEntityComponentSystem::runReadSystem( const CDrawSystemInfo& target, const ISystemContext& context ) const
{
	auto& system = static_cast<const IReadSystem&>( *target.System.Ptr() );
	system.RunGeneralDraw( context );
}

void CEntityComponentSystem::runDrawSystem( const CDrawSystemInfo& target, const ISystemContext& context ) const
{
	auto& system = static_cast<const IDrawSystem&>( *target.System.Ptr() );
	system.RunEntityListDraw( Entities( *target.TargetGroup ), context );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
