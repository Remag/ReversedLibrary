#pragma once
#include <Action.h>
#include <TemplateUtils.h>
#include <Invoke.h>

namespace Relib {

namespace RelibInternal {

template <class T>
const T& GetObjectType( const T& obj )
{
	return obj;
}

template <class T>
const T& GetObjectType( const T* ptr )
{
	return *ptr;
}

//////////////////////////////////////////////////////////////////////////

template <class Action, class Arg>
struct CIsInvokable {
	template <class T>
	static const T& CreateT();

	template <class TAction, class TArg>
	static decltype( CreateT<TAction>()( CreateT<TArg>() ), Types::TrueType() ) InvokeTest( int );

	template <class, class>
	static Types::FalseType InvokeTest( ... );

	typedef decltype( InvokeTest<Action, Arg>( 0 ) ) Result;
};

template <class Src, class Target>
struct CIsDereferencibleInto {
	template <class T>
	static const T& CreateT();

	template <class TSrc>
	static Types::IsSame<Target, typename Types::PureType<decltype( *CreateT<TSrc>() )>::Result> DerefTest( int );

	template <class>
	static Types::FalseType DerefTest( ... );

	typedef decltype( DerefTest<Src>( 0 ) ) Result;
};

template <class MemberType, class BaseType, class Arg>
struct CIsInvokable<MemberType BaseType::*, Arg> {
	static const bool isRelated = Types::IsSame<BaseType, Arg>::Result || Types::IsDerivedFrom<Arg, BaseType>::Result;
	static const bool isDereferencible = CIsDereferencibleInto<Arg, BaseType>::Result::Result;
	typedef Types::BoolType<isRelated || isDereferencible> Result;
};

template <class ReturnType, class BaseType, class Arg, class... Args>
struct CIsInvokable<ReturnType BaseType::*( Args... ), Arg> {
	typedef Types::IsConvertible<const Arg&, const BaseType&> Result;
};

template <class ReturnType, class BaseType, class Arg, class... Args>
struct CIsInvokable<ReturnType BaseType::*( Args... ) const, Arg> {
	typedef Types::IsConvertible<const Arg&, const BaseType&> Result;
};

//////////////////////////////////////////////////////////////////////////

template <class T, class ActionType>
decltype( auto ) doTryInvoke( const T& obj, const ActionType& action, Types::TrueType canInvokeMarker )
{
	canInvokeMarker;
	return Invoke( action, obj );
}

template <class T, class ActionType>
decltype( auto ) doTryInvoke( const T& obj, const ActionType&, Types::FalseType canInvokeMarker )
{
	canInvokeMarker;
	return obj;
}

template <class T, class ActionType>
decltype( auto ) TryInvoke( const T& obj, const ActionType& action )
{
	return doTryInvoke( obj, action, CIsInvokable<ActionType, T>::Result() );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////
// Less functions. Use operator< for comparion.
inline auto Less()
{
	return []( const auto& left, const auto& right )
		{ return left < right; };
}

inline auto LessPtr()
{
	return []( const auto& left, const auto& right )
		{ return *left < *right; };
}

template <class ActionType>
auto LessByAction( const ActionType& action )
{
	return [&]( const auto& left, const auto& right ) 
		{ return RelibInternal::TryInvoke( left, action ) < RelibInternal::TryInvoke( right, action ); };
}

//////////////////////////////////////////////////////////////////////////

// WeakLess functions. Use operator<= for comparion.
inline auto WeakLess()
{
	return []( const auto& left, const auto& right )
		{ return left <= right; };
}

inline auto WeakLessPtr()
{
	return []( const auto& left, const auto& right )
		{ return *left <= *right; };
}

template <class ActionType>
auto WeakLessByAction( const ActionType& action )
{
	return [&]( const auto& left, const auto& right ) 
		{ return RelibInternal::TryInvoke( left, action ) <= RelibInternal::TryInvoke( right, action ); };
}

//////////////////////////////////////////////////////////////////////////

// Greater functions. Use operator> for comparison.
inline auto Greater()
{
	return []( const auto& left, const auto& right ) 
		{ return left > right; };
}

inline auto GreaterPtr()
{
	return []( const auto& left, const auto& right ) 
		{ return *left > *right; };
}

template <class ActionType>
auto GreaterByAction( const ActionType& action )
{
	return [&]( const auto& left, const auto& right ) 
		{ return RelibInternal::TryInvoke( left, action ) > RelibInternal::TryInvoke( right, action ); };
}

//////////////////////////////////////////////////////////////////////////

// WeakGreater functions. Use operator>= for comparison.
inline auto WeakGreater()
{
	return []( const auto& left, const auto& right ) 
		{ return left >= right; };
}

inline auto WeakGreaterPtr()
{
	return []( const auto& left, const auto& right ) 
		{ return *left >= *right; };
}

template <class ActionType>
auto WeakGreaterByAction( const ActionType& action )
{
	return [&]( const auto& left, const auto& right ) 
		{ return RelibInternal::TryInvoke( left, action ) >= RelibInternal::TryInvoke( right, action ); };
}

//////////////////////////////////////////////////////////////////////////

// Equality functions. Use operator== for comparison.
inline auto Equal()
{
	return []( const auto& left, const auto& right ) 
		{ return left == right; };
}

inline auto EqualPtr()
{
	return []( const auto& left, const auto& right ) 
		{ return *left == *right; };
}

template <class ActionType>
auto EqualByAction( const ActionType& action )
{
	return [&]( const auto& left, const auto& right ) 
		{ return RelibInternal::TryInvoke( left, action ) == RelibInternal::TryInvoke( right, action ); };
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


