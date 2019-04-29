#pragma once
#include <Component.h>

namespace Relib {

class CEntityRef;
class CEntityConstRef;
class CEntityGroup;
struct CFullEntityData;
	//////////////////////////////////////////////////////////////////////////

// A central entity access point. Provides access to any component and gives ability to add and remove them.
class REAPI CConstEntity {
public:
	explicit CConstEntity( const CEntityGroup& group, int groupPos, CFullEntityData* _fullData ) :
		entityGroup( group ), entityGroupPos( groupPos ), fullData( _fullData ) {}
	
	CFullEntityData* GetFullData() const
		{ return fullData; }

	// Create a stable reference value that can be stored.
	CEntityConstRef CreateReference() const;

	// Component value getters. The entity must have the specified component.
	template <class T>
	const T& GetValue( const CComponent<T>& component ) const;
	// Check for component existence and return the value if it exists or nullptr otherwise.
	template <class T>
	const T* TryGetValue( const CComponent<T>& component ) const;

private:
	// Complete entity data.
	CFullEntityData* fullData;
	// Container with entities' component data.
	const CEntityGroup& entityGroup;
	// Entity position in the group.
	int entityGroupPos = NotFound;
};

//////////////////////////////////////////////////////////////////////////

// A central entity access point. Provides access to any component and gives ability to add and remove them.
class REAPI CEntity {
public:
	CEntity() = default;
	explicit CEntity( CEntityGroup& group, int groupPos, CFullEntityData* _fullData ) : 
		entityGroup( &group ), entityGroupPos( groupPos ), fullData( _fullData ) {}

	CFullEntityData* GetFullData() const
		{ return fullData; }

	// Create a stable reference value that can be stored.
	CEntityRef CreateReference();

	operator CConstEntity() const
		{ return CConstEntity( *entityGroup, entityGroupPos, fullData ); }

	// Component value getters. The entity must have the specified component.
	template <class T>
	T& GetValue( const CComponent<T>& component );
	template <class T>
	const T& GetValue( const CComponent<T>& component ) const
		{ return const_cast<CEntity*>( this )->GetValue( component ); }
	// Check for component existence and return the value if it exists or nullptr otherwise.
	template <class T>
	T* TryGetValue( const CComponent<T>& component );
	template <class T>
	const T* TryGetValue( const CComponent<T>& component ) const
		{ return const_cast<CEntity*>( this )->TryGetValue( component ); }

	// Entity container needs access to the entity group information.
	friend class CEntityContainer;

private:
	// Complete entity data.
	CFullEntityData* fullData;
	// Container with entities' component data.
	CEntityGroup* entityGroup;
	// Entity position in the group.
	int entityGroupPos = NotFound;

	CEntityGroup& getOwnerGroup()
		{ return *entityGroup; }
	int getGroupPos() const
		{ return entityGroupPos; }
};

//////////////////////////////////////////////////////////////////////////

// Entity information that includes its current generation.
struct CFullEntityData {
	CEntity Entity;
	int Generation = 0;
	int Id = NotFound;

	explicit CFullEntityData( int id ) : Id( id ) {}
};

//////////////////////////////////////////////////////////////////////////

template <class T>
const T& CConstEntity::GetValue( const CComponent<T>& component ) const
{
	return const_cast<CEntityGroup&>( entityGroup ).GetValue( component, entityGroupPos );
}

template <class T>
const T* CConstEntity::TryGetValue( const CComponent<T>& component ) const
{
	return const_cast<CEntityGroup&>( entityGroup ).TryGetValue( component, entityGroupPos );
}

//////////////////////////////////////////////////////////////////////////

template <class T>
T& CEntity::GetValue( const CComponent<T>& component )
{
	return entityGroup->GetValue( component, entityGroupPos );
}

template <class T>
T* CEntity::TryGetValue( const CComponent<T>& component )
{
	return entityGroup->TryGetValue( component, entityGroupPos );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

