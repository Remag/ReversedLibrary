#pragma once
#include <DynamicBitset.h>
#include <PersistentStorage.h>
#include <Map.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

enum TNamedComponentType {
	NCT_Bool,
	NCT_Int,
	NCT_Float,
	NCT_Double,
	NCT_Vec2,
	NCT_Vec3,
	NCT_IntVec2,
	NCT_IntVec3,
	NCT_AString,
	NCT_WString,
	NCT_CustomPrimitive,
	NCT_CustomComplex,
	NCT_EnumCount
};

namespace Types {

template <class T>
struct NamedComponentType {
	template<class T, class Enable = void> 
	struct CPrimitiveDetector {
		static const TNamedComponentType Result = NCT_CustomPrimitive;
	};
	template<class T>
	struct CPrimitiveDetector <T, typename Types::EnableIf<Types::IsClass<T>::Result>::Result> {
		static const TNamedComponentType Result = ( Types::HasTrivialMoveConstructor<T>::Result && Types::HasTrivialDestructor<T>::Result ) ? NCT_CustomPrimitive : NCT_CustomComplex;
	};

	static const TNamedComponentType Result = CPrimitiveDetector<T>::Result;
};

template <>
struct NamedComponentType<bool> {
	static const TNamedComponentType Result = NCT_Bool;
};

template <>
struct NamedComponentType<int> {
	static const TNamedComponentType Result = NCT_Int;
};

template <>
struct NamedComponentType<float> {
	static const TNamedComponentType Result = NCT_Float;
};

template <>
struct NamedComponentType<double> {
	static const TNamedComponentType Result = NCT_Double;
};

template <>
struct NamedComponentType<CVector2<float>> {
	static const TNamedComponentType Result = NCT_Vec2;
};

template <>
struct NamedComponentType<CVector3<float>> {
	static const TNamedComponentType Result = NCT_Vec3;
};

template <>
struct NamedComponentType<CVector2<int>> {
	static const TNamedComponentType Result = NCT_IntVec2;
};

template <>
struct NamedComponentType<CVector3<int>> {
	static const TNamedComponentType Result = NCT_IntVec3;
};

template <>
struct NamedComponentType<CString> {
	static const TNamedComponentType Result = NCT_AString;
};

template <>
struct NamedComponentType<CUnicodeString> {
	static const TNamedComponentType Result = NCT_WString;
};

}	// namespace Types.

//////////////////////////////////////////////////////////////////////////

// Component class represents a general type of an entity. 
// Entity of a certain general type can contain a restricted number of components.
class IComponentClass {
public:
	// Get a component class unique identifier.
	virtual int GetClassId() const = 0;
};

namespace RelibInternal {

// Component class global information.
struct CComponentClassInfo {
	int ClassUniqueId;
	int ComponentFreeId;
};

extern REAPI CMap<CStringView, CComponentClassInfo, CDefaultHash<CStringView>, CProcessHeap> ComponentClassNameToId;
extern REAPI CCriticalSection ComponentIdsSection;

template <class ComponentClass>
class CComponentClassInstance {
public:
	staticAssert( ( Types::IsDerivedFrom<ComponentClass, IComponentClass>::Result ) );

	static const ComponentClass& GetInstance()
		{ return getInstanceHolder().ClassInstance; }

	// Retrieve a component identifier and mark it as occupied.
	static int RetrieveNextFreeId();

	// Get unique component class identifier.
	static int GetUniqueClassId()
		{ return GetInstance().GetUniqueId(); }

private:
	struct CComponentClassHolder {
		// Instance of the component class.
		ComponentClass ClassInstance;
		int* FreeId = nullptr;

		explicit CComponentClassHolder( CStringView className );
	};

	static CComponentClassHolder& getInstanceHolder();
};

//////////////////////////////////////////////////////////////////////////

template <class ComponentClass>
CComponentClassInstance<ComponentClass>::CComponentClassHolder::CComponentClassHolder( CStringView className )
{
	CCriticalSectionLock idLock( ComponentIdsSection );
	auto& classInfo = ComponentClassNameToId.GetOrCreate( className, CComponentClassInfo{ ComponentClassNameToId.Size(), 0 } ).Value();
	FreeId = &classInfo.ComponentFreeId;
	ClassInstance.initComponentClass( classInfo.ClassUniqueId );
}

template <class ComponentClass>
typename CComponentClassInstance<ComponentClass>::CComponentClassHolder& CComponentClassInstance<ComponentClass>::getInstanceHolder()
{
	static CComponentClassHolder holder( typeid( ComponentClass ).name() );
	return holder;
}

template <class ComponentClass>
int CComponentClassInstance<ComponentClass>::RetrieveNextFreeId()
{
	const int result = ( *getInstanceHolder().FreeId )++;
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Get the unique identifier of a component class.
template <class ComponentClass>
int GetComponentClassId()
{
	return RelibInternal::CComponentClassInstance<ComponentClass>::GetUniqueClassId();
}

// Get the instance of a component class.
template <class ComponentClass>
const ComponentClass& GetComponentClassInstance()
{
	return RelibInternal::CComponentClassInstance<ComponentClass>::GetInstance();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

