#pragma once
#include <Redefs.h>
#include <BaseString.h>
#include <MemoryUtils.h>
#include <SafeCounters.h>
#include <StaticAllocators.h>
#include <Tuple.h>
#include <Action.h>
#include <ExternalObject.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Base creation function wrapper. Pointers to this class are stored in an external name base.
// This class is casted to the real template wrapper before use.
class CBaseObjectCreationFunction {
public:
	explicit CBaseObjectCreationFunction( int _objectSize ) : objectSize( _objectSize ) {}

	// Class must be polymorphic for dynamic_cast to work.
	virtual ~CBaseObjectCreationFunction() {}

	// Get the size of created object in bytes.
	int GetObjectSize() const
		{ return objectSize; }

private:
	// Size of created object.
	int objectSize;
};

//////////////////////////////////////////////////////////////////////////

// Actual function wrapper. Stores a function with template arguments as its parameters.
template <class ArgTuple>
class CObjectCreationFunction;

template <class... Args>
class CObjectCreationFunction<CTuple<Args...>> : public CBaseObjectCreationFunction {
public:
	typedef IAction<IExternalObject*( void*, Args... )> TCreateAction;
	typedef CTuple<Args...> TCreationArgsTuple;

	CObjectCreationFunction( CPtrOwner<TCreateAction> createFunc, int objectSize ) :
		CBaseObjectCreationFunction( objectSize ), createAction( move( createFunc ) ) {}

	// Call the creation function.
	IExternalObject* CreateObject( void* targetMemory, Args... args ) const
		{ return createAction->Invoke( targetMemory, forward<Args>( args )... ); }

private:
	// Pointer to creation action.
	CPtrOwner<TCreateAction> createAction;
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// An association between a class and its creation function.
// By default classes are associated with default constructor creation.
template <class T, class Enable = void>
struct CObjectCreationInfo {
	typedef CObjectCreationFunction<CTuple<>> TCreateFunction;
};

// Classes that inherit from IConstructible, use the function with the annotated constructor arguments.
template <class T>
struct CObjectCreationInfo<T, typename Types::EnableIf<Types::IsDerivedFrom<T, IBaseConstructable>::Result>::Result> {
	typedef CObjectCreationFunction<typename T::TConstructorArgsTuple> TCreateFunction;
};

//////////////////////////////////////////////////////////////////////////

// Action that creates an object using its constructor with the specified arguments.
template <class ClassType, class ArgTuple>
class CClassConstructionAction;

template <class ClassType, class... Args>
class CClassConstructionAction<ClassType, CTuple<Args...>> : public IAction<IExternalObject*( void*, Args... )> {
public:
	// Action invocation creates a class object using its constructor with correctly converted arguments.
	virtual IExternalObject* Invoke( void* targetMemory, Args... args ) const override final {
		static_assert( Types::HasConstructor<ClassType, Args...>::Result, "External object must be constructable with the specified arguments." );
		return ::new( targetMemory ) ClassType( forward<Args>( args )... ); 
	}
};

// Action that uses another arbitrary action to create an object.
template <class ActionType, class ArgTuple>
class CClassCreationAction;

template <class ActionType, class... Args>
class CClassCreationAction<ActionType, CTuple<Args...>> : public IAction<IExternalObject*( void*, Args... )> {
public:
	CClassCreationAction( ActionType _action ) : action( move( _action ) ) {}

	// Action invocation creates a class object using its constructor with correctly converted arguments.
	virtual IExternalObject* Invoke( void* targetMemory, Args... args ) const override final
		{ return doInvoke( targetMemory, forward<Args>( args )... ); }

private:
	ActionType action;

	IExternalObject* doInvoke( void* targetMemory, Args... args ) const;
};

//////////////////////////////////////////////////////////////////////////

template <class ActionType, class... Args>
IExternalObject* CClassCreationAction<ActionType, CTuple<Args...>>::doInvoke( void* targetMemory, Args... args ) const
{
	typedef Types::FunctionInfo<ActionType>::ReturnType TClassType;
	typedef Types::FunctionInfo<ActionType>::ArgsTuple TConstructorArgs;
	staticAssert( TConstructorArgs::Size() == sizeof...( Args ) );
	return ::new( targetMemory ) TClassType( Relib::Invoke( action, forward<Args>( args )... ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Return a creation function for an object with the given name.
// ObjectName must be a registered external class name.
REAPI const CBaseObjectCreationFunction* GetObjectCreationFunction( CUnicodePart objectName );
// Register an external name and a creation function for an object of a given type.
void REAPI RegisterObject( const type_info& objectInfo, CUnicodePart objectName, CPtrOwner<CBaseObjectCreationFunction, CProcessHeap> newFunction );
// Get the registered external name for an external object.
// Note that a single object can have several external names.
// In this case, the latest registered name is returned.
CUnicodeView REAPI GetExternalName( const IExternalObject& object );
// Check if the given name has a registered object associated with it.
bool REAPI IsExternalName( CUnicodePart name );

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

