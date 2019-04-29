#include <EntityContainer.h>
#include <ArrayBuffer.h>
#include <Entity.h>
#include <Reassert.h>
#include <Optional.h>
#include <ComponentGroup.h>
#include <EntityGroup.h>
#include <EntityInitializer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CEntityContainer::CEntityContainer()
{
}

CEntityContainer::~CEntityContainer()
{
}

void CEntityContainer::Empty()
{
	for( auto& group : entityGroups ) {
		group->Empty();
	}
	freeDataList.Empty();
	const auto entityCount = entityList.Size();
	for( int i = 0; i < entityCount; i++ ) {
		auto& entity = entityList[i];
		entity.Generation++;
		freeDataList.Add( &entity );
	}
}

CEntityGroup& CEntityContainer::GetEntityGroup( int groupIndex )
{
	return *entityGroups[groupIndex];
}

const CEntityGroup& CEntityContainer::GetEntityGroup( int groupIndex ) const
{
	return *entityGroups[groupIndex];
}

int CEntityContainer::MatchNextEntityGroup( int startIndex, const CComponentGroup& componentGroup ) const
{
	// TODO: trie-based matching (maybe).
	const auto& targetSet = componentGroup.GetComponentSet();
	const auto groupCount = entityGroups.Size();
	for( int i = startIndex; i < groupCount; i++ ) {
		const auto& compBitset = entityGroups[i]->GetComponentGroup().GetComponentSet();
		if( doesRightContainLeft( targetSet, compBitset ) ) {
			return i;
		}
	}

	return NotFound;
}

CFullEntityData& CEntityContainer::CreateEntity( const CComponentGroup& componentGroup )
{
	const auto data = createEntityData();
	auto& group = getOrCreateEntityGroup( componentGroup );
	const auto entityGroupPos = group.AddEntity( data );

	data->Entity = CEntity( group, entityGroupPos, data );
	return *data;
}

CFullEntityData& CEntityContainer::CreateEmptyEntity()
{
	return *createEntityData();
}

void CEntityContainer::ReturnEmptyEntity( CEntityInitializer&& initializer )
{
	returnEmptyEntity( initializer.emptyEntity );
}

void CEntityContainer::returnEmptyEntity( CFullEntityData& fullData )
{
	fullData.Generation++;
	freeDataList.Add( &fullData );
}

CFullEntityData& CEntityContainer::FillEntity( CEntityInitializer&& initializer )
{
	const auto entity = &initializer.emptyEntity;
	RelibInternal::CFilledEntityData filledData( initializer.initData );
	auto& group = getOrCreateEntityGroup( filledData.GetComponentGroup() );
	const auto entityGroupPos = group.InitializeEntity( entity, move( filledData ) );

	entity->Entity = CEntity( group, entityGroupPos, entity );
	return *entity;
}

bool CEntityContainer::doesRightContainLeft( const CDynamicBitSet<>& left, const CDynamicBitSet<>& right ) const
{
	return right.Size() >= left.Size() && right.Has( left );
}

bool CEntityContainer::isComponentSetEqual( const CDynamicBitSet<>& left, const CDynamicBitSet<>& right ) const
{
	return left.Size() == right.Size() && left == right;
}

CFullEntityData* CEntityContainer::createEntityData()
{
	if( !freeDataList.IsEmpty() ) {
		const auto result = freeDataList.Last();
		freeDataList.DeleteLast();
		return result;
	}

	const auto newId = entityList.Size();
	return &entityList.Add( newId );
}

CEntityGroup& CEntityContainer::getOrCreateEntityGroup( const CComponentGroup& componentGroup )
{
	const auto& targetSet = componentGroup.GetComponentSet();
	for( auto& group : entityGroups ) {
		const auto& compBitset = group->GetComponentGroup().GetComponentSet();
		if( isComponentSetEqual( targetSet, compBitset ) ) {
			return *group;
		}
	}

	return *entityGroups.Add( CreateOwner<CEntityGroup>( copy( componentGroup ) ) );
}

void CEntityContainer::DestroyEntity( CEntity entity )
{
	const auto fullData = entity.GetFullData();
	returnEmptyEntity( *fullData );

	const auto groupPos = entity.getGroupPos();
	auto& group = entity.getOwnerGroup();
	const auto lastGroupPos = group.Size() - 1;
	if( groupPos != lastGroupPos ) {
		const auto movedData = group.MoveEntity( lastGroupPos, groupPos );
		// Fix the global reference for the moved entity.
		entityList[movedData->Id].Entity = CEntity( group, groupPos, movedData );
	}
	group.DeleteLastEntity();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
