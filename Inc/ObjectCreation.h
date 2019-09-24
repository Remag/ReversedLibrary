#pragma once
#include <Redefs.h>
#include <ObjectCreationUtils.h>
#include <Serializable.h>
#include <Ptr.h>
#include <MemoryOwner.h>
#include <BaseStringPart.h>
#include <BaseString.h>

namespace Relib {

namespace RelibInternal {

template <class Allocator>
int GetSafeCounterSize()
{
	return CeilTo( sizeof( CSafeStrongRefCounter<Allocator> ), AllocatorAlignment );
}

inline void* OffsetMemory( void* ptr, int offset )
{
	BYTE* bytePtr = reinterpret_cast<BYTE*>( ptr );
	return bytePtr + offset;
}

template <class ObjectType, class... Args>
auto ConvertCreationFunction( const CBaseObjectCreationFunction* baseFunction )
{
	typedef RelibInternal::CObjectCreationInfo<ObjectType>::TCreateFunction TCreateFunction;
	static_assert( TCreateFunction::TCreationArgsTuple::Size() == sizeof...( Args ), "External object cannot be created with the specified number of arguments." );
	auto function = static_cast<const TCreateFunction*>( baseFunction );
	// Check that the dynamic type of the function is correctly represented by ObjectType.
	assert( dynamic_cast<const TCreateFunction*>( baseFunction ) == function );
	return function;
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Create a serializable object. 
// Object name needs to be registered with RegisterExternalObject.
template<class ObjectType, class Allocator = CRuntimeHeap, class... CreationArgs>
CSharedPtr<ObjectType, Allocator> CreateSharedObject( CUnicodePart objectName, CreationArgs&&... args )
{
	return CreateSharedObject<ObjectType, Allocator>( GetObjectCreationFunction( objectName ), forward<CreationArgs>( args )... );
}

template<class ObjectType, class Allocator = CRuntimeHeap, class... CreationArgs>
CSharedPtr<ObjectType, Allocator> CreateSharedObject( const CBaseObjectCreationFunction* baseFunction, CreationArgs&&... args )
{
	// Deduce an appropriate creation function from ObjectType.
	auto function = RelibInternal::ConvertCreationFunction<ObjectType, CreationArgs...>( baseFunction );

	// Allocate the object with an embedded reference counter and construct it using the function.
	const int counterSize = RelibInternal::GetSafeCounterSize<Allocator>();
	CMemoryOwner<Allocator> objectMemory( Allocator::Allocate( function->GetObjectSize() + counterSize ) );
	IExternalObject* objectPtr = function->CreateObject( RelibInternal::OffsetMemory( objectMemory.Ptr(), counterSize ), forward<CreationArgs>( args )... );

	// Return the correct object type.
	ObjectType* castedPtr = static_cast<ObjectType*>( objectPtr );
	assert( dynamic_cast<ObjectType*>( objectPtr ) == castedPtr );
	// Create a new shared pointer.
	auto newObject = CSharedPtr<ObjectType, Allocator>::CreateNewPtr( castedPtr, objectMemory.Ptr() );
	assert( newObject != nullptr );
	objectMemory.Detach();
	return move( newObject );
}

template<class ObjectType, class Allocator, class... CreationArgs>
CSharedPtr<ObjectType, Allocator> AllocateSharedObject( CUnicodePart objectName, Allocator& allocator, CreationArgs&&... args )
{
	return AllocateSharedObject<ObjectType, Allocator>( GetObjectCreationFunction( objectName ), allocator, forward<CreationArgs>( args )... );
}

template<class ObjectType, class Allocator, class... CreationArgs>
CSharedPtr<ObjectType, Allocator> AllocateSharedObject( const CBaseObjectCreationFunction* baseFunction, Allocator& allocator, CreationArgs&&... args )
{
	// Deduce an appropriate creation function from ObjectType.
	auto function = RelibInternal::ConvertCreationFunction<ObjectType, CreationArgs...>( baseFunction );

	const int counterSize = RelibInternal::GetSafeCounterSize<Allocator>();
	CMemoryOwner<Allocator> objectMemory( allocator.Allocate( function->GetObjectSize() + counterSize ), allocator );
	// Construction arguments are passed as constant references, they are casted back to required references in the creation function.
	IExternalObject* objectPtr = function->CreateObject( RelibInternal::OffsetMemory( objectMemory.Ptr(), counterSize ), forward<CreationArgs>( args )... );
	// Return the correct object type.
	ObjectType* castedPtr = static_cast<ObjectType*>( objectPtr );
	assert( dynamic_cast<ObjectType*>( objectPtr ) == castedPtr );
	// Create a new shared pointer.
	auto newObject = CSharedPtr<ObjectType, Allocator>::CreateNewPtr( castedPtr, objectMemory.Ptr(), allocator );
	assert( newObject != nullptr );
	objectMemory.Detach();
	return move( newObject );
}

// Create a unique object.
// Object name needs to be registered with RegisterExternalObject.
template <class ObjectType, class Allocator = CRuntimeHeap, class... CreationArgs>
CPtrOwner<ObjectType, Allocator> CreateUniqueObject( CUnicodePart objectName, CreationArgs&&... args )
{
	return CreateUniqueObject<ObjectType, Allocator>( GetObjectCreationFunction( objectName ), forward<CreationArgs>( args )... );
}

template <class ObjectType, class Allocator = CRuntimeHeap, class... CreationArgs>
CPtrOwner<ObjectType, Allocator> CreateUniqueObject( const CBaseObjectCreationFunction* baseFunction, CreationArgs&&... args )
{
	// Deduce an appropriate creation function from ObjectType.
	auto function = RelibInternal::ConvertCreationFunction<ObjectType, CreationArgs...>( baseFunction );

	CMemoryOwner<Allocator> objectMemory( Allocator::Allocate( function->GetObjectSize() ) );
	// Construction arguments are passed as constant references, they are casted back to required references in the creation function.
	IExternalObject* objectPtr = function->CreateObject( objectMemory.Ptr(), forward<CreationArgs>( args )... );
	// Return the correct object type.
	ObjectType* castedPtr = static_cast<ObjectType*>( objectPtr );
	assert( dynamic_cast<ObjectType*>( objectPtr ) == castedPtr );
	CPtrOwner<ObjectType, Allocator> newObject( castedPtr );
	assert( newObject != 0 );
	objectMemory.Detach();
	return move( newObject );
}

template <class ObjectType, class Allocator, class... CreationArgs>
CPtrOwner<ObjectType, Allocator> AllocateUniqueObject( CUnicodePart objectName, Allocator& allocator, CreationArgs&&... args )
{
	return AllocateUniqueObject<ObjectType, Allocator>( GetObjectCreationFunction( objectName ), allocator, forward<CreationArgs>( args )... );
}

template <class ObjectType, class Allocator, class... CreationArgs>
CPtrOwner<ObjectType, Allocator> AllocateUniqueObject( const CBaseObjectCreationFunction* baseFunction, Allocator& allocator, CreationArgs&&... args )
{
	// Deduce an appropriate creation function from ObjectType.
	auto function = RelibInternal::ConvertCreationFunction<ObjectType, CreationArgs...>( baseFunction );

	CMemoryOwner<Allocator> objectMemory( allocator.Allocate( function->GetObjectSize() ), allocator );
	// Construction arguments are passed as constant references, they are casted back to required references in the creation function.
	IExternalObject* objectPtr = function->CreateObject( objectMemory.Ptr(), forward<CreationArgs>( args )... );
	// Return the correct object type.
	ObjectType* castedPtr = static_cast<ObjectType*>( objectPtr );
	assert( dynamic_cast<ObjectType*>( objectPtr ) == castedPtr );
	CPtrOwner<ObjectType, Allocator> newObject( castedPtr, allocator );
	assert( newObject != nullptr );
	
	objectMemory.Detach();
	return move( newObject );
}

//////////////////////////////////////////////////////////////////////////

// Class that associates a name with an arbitrary class. This class can be created with a call to CreateUniqueObject afterwards.
// ConstructorArgs are the argument types of a constructor to use.
// They must be ref qualified exactly as in the target constructor.
template <class ClassType>
class CExternalNameConstructor {
public:
	staticAssert( ( Types::IsDerivedFrom<ClassType, IExternalObject>::Result ) );

	// Constructor that registers the default function.
	// Default function uses ClassType's constructor.
	explicit CExternalNameConstructor( CUnicodePart className );
	~CExternalNameConstructor();

private:
	CUnicodeString className;
	const type_info* classInfo;
};

//////////////////////////////////////////////////////////////////////////

template <class ClassType>
CExternalNameConstructor<ClassType>::CExternalNameConstructor( CUnicodePart _className ) :
	className( _className )
{
	classInfo = &typeid( ClassType );
	const int objectSize = sizeof( ClassType );
	typedef RelibInternal::CObjectCreationInfo<ClassType>::TCreateFunction TCreateFunction;
	typedef typename TCreateFunction::TCreationArgsTuple TArgsTuple;
	auto classCreateAction = CreateOwner<RelibInternal::CClassConstructionAction<ClassType, TArgsTuple>>();
	auto creationFunction = CreateOwner<TCreateFunction, CProcessHeap>( move( classCreateAction ), objectSize );
	RegisterObject( *classInfo, className, move( creationFunction ) );
}

template<class ClassType>
CExternalNameConstructor<ClassType>::~CExternalNameConstructor()
{
	UnregisterObject( *classInfo, className );
}

//////////////////////////////////////////////////////////////////////////

// Class that associates a name with an arbitrary class. This class can be created with a call to CreateUniqueObject afterwards.
// A specified action is used to create the class object.
// The action must return the constructed object by value.
class CExternalNameCreator {
public:
	// Register a given creation function.
	template <class ActionType>
	explicit CExternalNameCreator( CUnicodePart className, ActionType action );
	~CExternalNameCreator();

private:
	CUnicodeString className;
	const type_info* classInfo;
};

//////////////////////////////////////////////////////////////////////////

template <class ActionType>
CExternalNameCreator::CExternalNameCreator( CUnicodePart _className, ActionType action ) :
	className( _className )
{
	typedef Types::FunctionInfo<ActionType>::ReturnType TClassType;
	classInfo = &typeid( TClassType );
	typedef Types::FunctionInfo<ActionType>::ArgsTuple TArgTuple;
	const int objectSize = sizeof( TClassType );
	auto classCreateAction = CreateOwner<RelibInternal::CClassCreationAction<ActionType, TArgTuple>>( move( action ) );
	auto creationFunction = CreateOwner<CObjectCreationFunction<TArgTuple>, CProcessHeap>( move( classCreateAction ), objectSize );
	RegisterObject( *classInfo, className, move( creationFunction ) );
}

inline CExternalNameCreator::~CExternalNameCreator()
{
	UnregisterObject( *classInfo, className );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.