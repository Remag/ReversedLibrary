#pragma once
#include <Redefs.h>
#include <BaseString.h>
#include <ComponentUtils.h>
#include <Pair.h>
#include <Map.h>
 
namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A weakly typed component.
class CNamedInlineComponent {
public:
	// Constructors have to be public to be accessible from CMap.
	CNamedInlineComponent() = default;
	template <class ComponentClass>
	CNamedInlineComponent( TNamedComponentType compType, int typeSize, int typeAlignment, const ComponentClass& compClass ) : 
		type( compType ), id( RelibInternal::CComponentClassInstance<ComponentClass>::RetrieveNextFreeId() ), 
		size( typeSize ), alignment( typeAlignment ), componentClass( &compClass ) {}
	CNamedInlineComponent( TNamedComponentType compType, int typeSize, int typeAlignment, const IComponentClass& compClass, int componentId ) : 
		type( compType ), id( componentId ), size( typeSize ), alignment( typeAlignment ), componentClass( &compClass ) {}

	// Named component creation and retrieval.
	// Get method returns null if no component with the given name exists.
	static CNamedInlineComponent* Get( CStringPart name );
	template <class ComponentClass>
	static const CNamedInlineComponent* GetById( int componentId );

	template <class ComponentClass>
	static CNamedInlineComponent GetOrCreate( CStringPart name, TNamedComponentType type );
	template <class ComponentClass>
	static CNamedInlineComponent GetOrCreate( CStringPart name, int componentSize, int componentAlignment );

	template <class ComponentClass>
	static CNamedInlineComponent Create( CStringPart name, TNamedComponentType type );
	template <class ComponentClass>
	static CNamedInlineComponent Create( CStringPart name, int componentSize, int componentAlignment );
	template <class T, class ComponentClass>
	static CNamedInlineComponent Create( CStringPart name );

	// Associate a name with an existing component.
	template <class T, class ComponentClass>
	static CNamedInlineComponent AddExisting( CStringPart name, const CInlineComponent<T, ComponentClass>& component );
	template <class ComponentClass>
	static CNamedInlineComponent AddExisting( CStringPart name, TNamedComponentType type, const ComponentClass& classInstance, int componentId );

	CStringView Name() const
		{ return name; }
	// Identifier and size of the component.
	int GetClassId() const
		{ return componentClass->GetClassId(); }
	int GetId() const
		{ return id; }
	int GetSize() const
		{ return size; }
	int GetAlignment() const
		{ return alignment; }
	TNamedComponentType GetType() const
		{ return type; }

private:
	const IComponentClass* componentClass = nullptr;
	CStringView name;
	TNamedComponentType type;
	int id;
	int size;
	int alignment;

	void setComponentName( CStringView newValue )
		{ name = newValue; }

	template <class ComponentClass>
	static CNamedInlineComponent getOrCreate( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment );
	template <class ComponentClass>
	static CNamedInlineComponent create( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment );
	template <class ComponentClass>
	static CNamedInlineComponent setNamedComponent( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment, const ComponentClass& classInstance );
	template <class ComponentClass>
	static void setComponentIdValue( CNamedInlineComponent component );
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {
	// Named component database.
	extern REAPI CMap<CString, CNamedInlineComponent, CDefaultHash<CString>, CProcessHeap> NamedInlineComponents;
	// Named component database keyed by a class id and component id pair.
	extern REAPI CMap<CPair<int>, CNamedInlineComponent, CDefaultHash<CPair<int>>, CProcessHeap> NamedInlineComponentIds;
	// Component size and alignment dictionary.
	extern REAPI CEnumDictionary<TNamedComponentType, NCT_EnumCount, CPair<unsigned>> NamedComponentSizeDict;
}

//////////////////////////////////////////////////////////////////////////

inline CNamedInlineComponent* CNamedInlineComponent::Get( CStringPart name )
{
	return RelibInternal::NamedInlineComponents.Get( name );
}

template <class ComponentClass>
const CNamedInlineComponent* CNamedInlineComponent::GetById( int componentId )
{
	const auto classId = GetComponentClassInstance<ComponentClass>().GetUniqueId(); 
	const auto idPair = CreatePair( classId, componentId );
	return RelibInternal::NamedInlineComponentIds.Get( idPair );
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::GetOrCreate( CStringPart name, int componentSize, int componentAlignment )
{
	return getOrCreate<ComponentClass>( name, NCT_CustomComplex, componentSize, componentAlignment );
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::GetOrCreate( CStringPart name, TNamedComponentType type )
{
	staticAssert( NCT_EnumCount == 12 );
	assert( type != NCT_CustomComplex && type != NCT_CustomPrimitive );
	const auto sizeAlign = RelibInternal::NamedComponentSizeDict[type];
	return getOrCreate<ComponentClass>( name, type, numeric_cast<int>( sizeAlign.First ), numeric_cast<int>( sizeAlign.Second ) );
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::getOrCreate( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment )
{
	assert( !name.IsEmpty() );
	const auto& classInstance = GetComponentClassInstance<ComponentClass>(); 
	auto& componentData = RelibInternal::NamedInlineComponents.GetOrCreate( name, type, componentSize, componentAlignment, classInstance );
	auto& resultValue = componentData.Value();
	if( resultValue.Name().IsEmpty() ) {
		resultValue.setComponentName( componentData.Key() );
		setComponentIdValue<ComponentClass>( resultValue );
	}
	return resultValue;
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::Create( CStringPart name, int componentSize, int componentAlignment )
{
	return create<ComponentClass>( name, NCT_CustomComplex, componentSize, componentAlignment );
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::Create( CStringPart name, TNamedComponentType type )
{
	staticAssert( NCT_EnumCount == 12 );
	assert( type != NCT_CustomComplex && type != NCT_CustomPrimitive );
	const auto sizeAlign = RelibInternal::NamedComponentSizeDict[type];
	return create<ComponentClass>( name, type, numeric_cast<int>( sizeAlign.First ), numeric_cast<int>( sizeAlign.Second ) );
}

template <class T, class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::Create( CStringPart name )
{
	const auto type = Types::NamedComponentType<T>::Result;
	return create<ComponentClass>( name, type, sizeof( T ), alignof( T ) );
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::create( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment )
{
	assert( !RelibInternal::NamedInlineComponents.Has( name ) );
	const auto& classInstance = GetComponentClassInstance<ComponentClass>(); 
	const auto newData = setNamedComponent( name, type, componentSize, componentAlignment, classInstance );
	setComponentIdValue<ComponentClass>( newData );
	return newData;
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::setNamedComponent( CStringPart name, TNamedComponentType type, int componentSize, int componentAlignment, const ComponentClass& classInstance )
{
	auto& newData = RelibInternal::NamedInlineComponents.Set( name, type, componentSize, componentAlignment, classInstance );
	newData.Value().setComponentName( newData.Key() );
	return newData.Value();
}

template <class ComponentClass>
void CNamedInlineComponent::setComponentIdValue( CNamedInlineComponent component )
{
	const auto classId = GetComponentClassInstance<ComponentClass>().GetUniqueId(); 
	const auto idPair = CreatePair( classId, component.GetId() );
	RelibInternal::NamedInlineComponentIds.Set( idPair, component );
}

template <class T, class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::AddExisting( CStringPart name, const CInlineComponent<T, ComponentClass>& component )
{
	const auto type = component.GetType();
	const auto& classInstance = GetComponentClassInstance<ComponentClass>(); 
	auto& newData = RelibInternal::NamedInlineComponents.Set( name, type, sizeof( T ), alignof( T ), classInstance, component.GetId() );
	newData.Value().setComponentName( newData.Key() );
	setComponentIdValue<ComponentClass>( newData.Value() );
	return newData.Value();
}

template <class ComponentClass>
CNamedInlineComponent CNamedInlineComponent::AddExisting( CStringPart name, TNamedComponentType type, const ComponentClass& classInstance, int componentId )
{
	const auto sizeAlign = RelibInternal::NamedComponentSizeDict[type];
	auto& newData = RelibInternal::NamedInlineComponents.Set( name, type, numeric_cast<int>( sizeAlign.First ), numeric_cast<int>( sizeAlign.Second ), classInstance, componentId );
	newData.Value().setComponentName( newData.Key() );
	const auto classId = classInstance.GetClassId();
	const auto idPair = CreatePair( classId, componentId );
	RelibInternal::NamedInlineComponentIds.Set( idPair, newData.Value() );
	return newData.Value();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

