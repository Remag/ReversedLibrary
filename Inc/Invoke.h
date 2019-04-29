#pragma once
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Invoke a method pointer.
template <class ObjectType, class BaseType, class MethodType, class... MethodArgs, class... CallArgs>
auto doInvoke( Types::TrueType methodMarker, MethodType BaseType::*action, ObjectType&& o, CallArgs&&... args )
{
	methodMarker;
	typedef Types::Conditional<Types::IsConstRef<ObjectType>::Result, const BaseType&, BaseType&>::Result TTargetBaseType;
	return RelibInternal::doMethodInvoke( Types::IsConvertible<ObjectType, TTargetBaseType>(), action, forward<ObjectType>( o ), forward<CallArgs>( args )... );
}

// Invoke a member pointer.
template <class ObjectType, class BaseType, class MemberType, class... MethodArgs, class... CallArgs>
decltype( auto ) doInvoke( Types::FalseType methodMarker, MemberType BaseType::*member, ObjectType&& o, CallArgs&&... )
{
	staticAssert( sizeof...( CallArgs ) == 0 );
	methodMarker;
	typedef Types::PureType<ObjectType>::Result TPureObjectType;
	static const bool isInvokable = Types::IsSame<BaseType, TPureObjectType>::Result || Types::IsDerivedFrom<TPureObjectType, BaseType>::Result;
	return RelibInternal::doMemberInvoke( Types::BoolType<isInvokable>(), member, forward<ObjectType>( o ) );
}

// Perform member invocation on objects that are directly related.
template <class ObjectType, class BaseType, class MemberType>
decltype( auto ) doMemberInvoke( Types::TrueType derivedMarker, MemberType BaseType::*member, ObjectType&& o )
{
	derivedMarker;
	return o.*member;
}

// Perform member invocation on other objects.
template <class ObjectType, class BaseType, class MemberType>
decltype( auto ) doMemberInvoke( Types::FalseType derivedMarker, MemberType BaseType::*member, ObjectType&& o )
{
	derivedMarker;
	return ( *o ).*member;
}

// Perform method invocation on objects that are directly related.
template <class ObjectType, class BaseType, class ReturnType, class... FunctionArgs, class... CallArgs>
ReturnType doMethodInvoke( Types::TrueType derivedMarker, ReturnType ( BaseType::*action )( FunctionArgs... ), ObjectType& o, CallArgs&&... args )
{
	derivedMarker;
	return ( static_cast<BaseType&>( o ).*action )( forward<CallArgs>( args )... );
}

template <class ObjectType, class BaseType, class ReturnType, class... FunctionArgs, class... CallArgs>
ReturnType doMethodInvoke( Types::TrueType derivedMarker, ReturnType ( BaseType::*action )( FunctionArgs... ) const, const ObjectType& o, CallArgs&&... args )
{
	derivedMarker;
	return ( static_cast<const BaseType&>( o ).*action )( forward<CallArgs>( args )... );
}

// Perform method invocation on other objects.
template <class ObjectType, class BaseType, class ReturnType, class... FunctionArgs, class... CallArgs>
ReturnType doMethodInvoke( Types::FalseType derivedMarker, ReturnType ( BaseType::*action )( FunctionArgs... ), const ObjectType& o, CallArgs&&... args )
{
	derivedMarker;
	return ( static_cast<BaseType&>( *o ).*action )( forward<CallArgs>( args )... );
}

template <class ObjectType, class BaseType, class ReturnType, class... FunctionArgs, class... CallArgs>
ReturnType doMethodInvoke( Types::FalseType derivedMarker, ReturnType ( BaseType::*action )( FunctionArgs... ) const, const ObjectType& o, CallArgs&&... args )
{
	derivedMarker;
	return ( static_cast<const BaseType&>( *o ).*action )( forward<CallArgs>( args )... );
}

}	// namespace RelibInternal.

// A unified callable invocation.
// Performs calls on function pointers, member pointers and method pointers.
//////////////////////////////////////////////////////////////////////////

// Function pointers.
template <class ReturnType, class... FunctionArgs, class... CallArgs>
ReturnType Invoke( ReturnType action( FunctionArgs... ), CallArgs&&... args )
{
	return action( forward<CallArgs>( args )... );
}

// Class data members.
template <class ObjectType, class BaseType, class MemberType, class... CallArgs>
decltype( auto ) Invoke( MemberType BaseType::*member, ObjectType&& o, CallArgs&&... args )
{
	return RelibInternal::doInvoke( Types::IsMethodPtr<decltype( member )>(), member, forward<ObjectType>( o ), forward<CallArgs>( args )... );
}

// Arbitrary callables.
template <class Callable, class... Args>
auto Invoke( Callable& callable, Args&&... args )
{
	return callable( forward<Args>( args )... );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

