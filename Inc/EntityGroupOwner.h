#pragma once
#include <EntityContainer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Entity group that is not associated with a system.
// This class deletes the group from the ECS on destruction.
class CEntityGroupOwner {
public:
	CEntityGroupOwner() = default;
	CEntityGroupOwner( CEntityContainer& _container, int _groupId ) : container( &_container ), groupId( _groupId ) {}
	CEntityGroupOwner( CEntityGroupOwner&& other ) : container( other.container ), groupId( other.groupId ) { other.groupId = NotFound; }
	CEntityGroupOwner& operator=( CEntityGroupOwner&& other );
	~CEntityGroupOwner()
		{ removeEntityGroup(); }

private:
	CEntityContainer* container;
	int groupId = NotFound;

	void removeEntityGroup();

	// Copying is prohibited.
	CEntityGroupOwner( CEntityGroupOwner& ) = delete;
	void operator=( CEntityGroupOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

inline CEntityGroupOwner& CEntityGroupOwner::operator=( CEntityGroupOwner&& other )
{
	swap( groupId, other.groupId );
	swap( container, other.container );

	return *this;
}

inline void CEntityGroupOwner::removeEntityGroup()
{
	if( groupId != NotFound ) { 
		container->RemoveEntityGroup( groupId );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

