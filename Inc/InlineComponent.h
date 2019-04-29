#pragma once
#include <Reutils.h>
#include <TemplateUtils.h>
#include <NamedInlineComponent.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A strongly typed component for use with inline entities. All components of this type should be defined as global variables.
template <class T, class ComponentClass>
class CInlineComponent {
public:
	CInlineComponent() : id( RelibInternal::CComponentClassInstance<ComponentClass>::RetrieveNextFreeId() ) {}
	// Create a component with a name. Name is registered among other named components.
	explicit CInlineComponent( CStringView name ) : id( CNamedInlineComponent::Create<T, ComponentClass>( name ).GetId() ) {}

	int GetId() const
		{ return id; }

	static TNamedComponentType GetType()
		{ return Types::NamedComponentType<T>::Result; }
		static int GetSize()
		{ return sizeof( T ); }
	static int GetAlignment()
		{ return alignof( T ); }
	
	static int GetClassId()
		{ return RelibInternal::CComponentClassInstance<ComponentClass>::GetUniqueClassId(); }

private:
	// Unique identifier.
	int id;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

