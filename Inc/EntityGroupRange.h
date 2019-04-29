#pragma once
#include <EntityContainer.h>
#include <EntityRange.h>
#include <EntityGroup.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for iteration of entities matching a given set of components.
class CBaseEntityGroupRange {
public:
	CBaseEntityGroupRange( const CEntityContainer& _container, const CComponentGroup& _components ) :
		container( _container ), components( _components ), groupPos( NotFound ) {}
	CBaseEntityGroupRange( const CEntityContainer& _container, const CComponentGroup& _components, int _groupPos ) :
		container( _container ), components( _components ), groupPos( _groupPos ) {}
	
	bool operator!=( CBaseEntityGroupRange other ) const
		{ return groupPos != other.groupPos; }

protected:
	CEntityConstRange getCurrentEntities() const
		{ return container.GetEntityGroup( groupPos ).Entities(); }
	CEntityRange getCurrentEntities()
		{ return const_cast<CEntityContainer&>( container ).GetEntityGroup( groupPos ).Entities(); }
	void setNextEntityGroup()
		{ groupPos = getNextEntityGroup( groupPos + 1 ); }
	int getNextEntityGroup( int startPos ) const
		{ return container.MatchNextEntityGroup( startPos, components ); }

	const CEntityContainer& getContainer() const
		{ return container; }
	CEntityContainer& getContainer()
		{ return const_cast<CEntityContainer&>( container ); }
	const CComponentGroup& getComponents() const
		{ return components; }
	int getGroupPos() const
		{ return groupPos; }

private:
	const CEntityContainer& container;
	const CComponentGroup& components;
	int groupPos;
};

//////////////////////////////////////////////////////////////////////////

class CEntityGroupConstRange : public CBaseEntityGroupRange {
public:
	using CBaseEntityGroupRange::CBaseEntityGroupRange;

	void operator++()
		{ setNextEntityGroup(); }
	CEntityConstRange operator*() const
		{ return getCurrentEntities(); }
	
	CEntityGroupConstRange begin() const
		{ return CEntityGroupConstRange( getContainer(), getComponents(), getNextEntityGroup( 0 ) ); }
	CEntityGroupConstRange end() const
		{ return CEntityGroupConstRange( getContainer(), getComponents(), NotFound ); }
};

//////////////////////////////////////////////////////////////////////////

class CEntityGroupRange : public CBaseEntityGroupRange {
public:
	CEntityGroupRange( CEntityContainer& _container, const CComponentGroup& _components ) : CBaseEntityGroupRange( _container, _components ) {}
	CEntityGroupRange( CEntityContainer& _container, const CComponentGroup& _components, int _groupPos ) : CBaseEntityGroupRange( _container, _components, _groupPos ) {}

	void operator++()
		{ setNextEntityGroup(); }
	CEntityRange operator*()
		{ return getCurrentEntities(); }
	
	CEntityGroupRange begin()
		{ return CEntityGroupRange( getContainer(), getComponents(), getNextEntityGroup( 0 ) ); }
	CEntityGroupRange end()
		{ return CEntityGroupRange( getContainer(), getComponents(), NotFound ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

