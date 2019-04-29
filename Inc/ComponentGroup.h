#pragma once
#include <Component.h>
#include <Redefs.h>
#include <StaticArray.h>
#include <DynamicBitset.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A collection of components which will form an entity group.
class CComponentGroup {
public:
	CComponentGroup() = default;
	CComponentGroup( const CComponentGroup& other, const CExplicitCopyTag& ) : components( copy( other.components ) ), componentIdSet( copy( other.componentIdSet ) ) {}
	template <class Component, class... Components>
	explicit CComponentGroup( const Component& first, const Components&... _components );

	void Empty();

	void Add( const CBaseComponent& component );

	const CDynamicBitSet<>& GetComponentSet() const
		{ return componentIdSet; }
	CArrayView<const CBaseComponent*> GetComponents() const
		{ return components; }

private:
	CArray<const CBaseComponent*> components;
	CDynamicBitSet<> componentIdSet;

	template <class FirstComponent, class SecondComponent, class... RestComponents>
	int findComponentCount( const FirstComponent& component, const SecondComponent& second, const RestComponents&... rest ) const;
	int findComponentCount( const CBaseComponent& component ) const;
	int findComponentCount( const CComponentGroup& group ) const;

	void fillComponentIds( const CComponentGroup& group );
	void fillComponentIds( const CBaseComponent& component );

	template <class FirstComponent, class SecondComponent, class... RestComponents>
	void fillComponentIds( const FirstComponent& first, const SecondComponent& second, const RestComponents&... rest );
};

//////////////////////////////////////////////////////////////////////////

template <class Component, class... Components>
CComponentGroup::CComponentGroup( const Component& first, const Components&... _components )
{
	components.ReserveBuffer( findComponentCount( first, _components... ) );
	fillComponentIds( first, _components... );
}

inline int CComponentGroup::findComponentCount( const CComponentGroup& group ) const
{
	return group.components.Size();
}

inline int CComponentGroup::findComponentCount( const CBaseComponent& ) const
{
	return 1;
}

template <class FirstComponent, class SecondComponent, class... RestComponents>
int CComponentGroup::findComponentCount( const FirstComponent& component, const SecondComponent& second, const RestComponents&... rest ) const
{
	return findComponentCount( component ) + findComponentCount( second, rest... );
}

inline void CComponentGroup::Empty()
{
	components.Empty();
	componentIdSet.Empty();
}

inline void CComponentGroup::Add( const CBaseComponent& component )
{
	const auto newId = component.GetComponentId();
	if( !componentIdSet.Has( newId ) ) {
		components.Add( &component );
		componentIdSet |= newId;
	}
}

inline void CComponentGroup::fillComponentIds( const CBaseComponent& component )
{
	const auto newId = component.GetComponentId();
	if( !componentIdSet.Has( newId ) ) {
		components.AddWithinCapacity( &component );
		componentIdSet |= newId;
	}
}

inline void CComponentGroup::fillComponentIds( const CComponentGroup& group )
{
	for( auto componentPtr : group.components ) {
		fillComponentIds( *componentPtr );
	}
}

template <class FirstComponent, class SecondComponent, class... RestComponents>
void CComponentGroup::fillComponentIds( const FirstComponent& first, const SecondComponent& second, const RestComponents&... rest )
{
	fillComponentIds( first );
	fillComponentIds( second, rest... );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
