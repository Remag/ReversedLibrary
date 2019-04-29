#pragma once
#include <Entity.h>
#include <EntityGroup.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Entity group iteration support.
class CEntityConstRange {
public:
	CEntityConstRange( const CEntityGroup& _group, int index, int count ) : group( _group ), entityIndex( index ), entityCount( count ) {}

	void operator++()
		{ entityIndex = getNextTakenIndex( entityIndex + 1 ); }
	CConstEntity operator*() const
		{ return CConstEntity{ group, entityIndex, group.GetEntityData( entityIndex ) }; }
	bool operator!=( CEntityConstRange other ) const
		{ return entityIndex != other.entityIndex; }

	CEntityConstRange begin() const
		{ return CEntityConstRange( group, getNextTakenIndex( entityIndex ), entityCount ); }
	CEntityConstRange end() const
		{ return CEntityConstRange( group, entityCount, entityCount ); }

private:
	const CEntityGroup& group;
	int entityIndex;
	int entityCount;

	int getNextTakenIndex( int index ) const;
};

//////////////////////////////////////////////////////////////////////////

class CEntityRange {
public:
	CEntityRange( CEntityGroup& _group, int index, int count ) : group( _group ), entityIndex( index ), entityCount( count ) {}

	void operator++()
		{ entityIndex = getNextTakenIndex( entityIndex + 1 ); }
	CEntity operator*() const
		{ return CEntity{ group, entityIndex, group.GetEntityData( entityIndex ) }; }
	bool operator!=( CEntityRange other ) const
		{ return entityIndex != other.entityIndex; }

	CEntityRange begin()
		{ return CEntityRange( group, getNextTakenIndex( entityIndex ), entityCount ); }
	CEntityRange end() 
		{ return CEntityRange( group, entityCount, entityCount ); }

private:
	CEntityGroup& group;
	int entityIndex;
	int entityCount;

	int getNextTakenIndex( int index ) const;
};

//////////////////////////////////////////////////////////////////////////

inline int CEntityConstRange::getNextTakenIndex( int index ) const
{
	return index;
}

inline int CEntityRange::getNextTakenIndex( int index ) const
{
	return index;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

