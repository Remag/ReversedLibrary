#pragma once
#include <VarArgsUtils.h>
#include <Remath.h>
#include <ExternalObject.h>

namespace Relib {

namespace Types {

//////////////////////////////////////////////////////////////////////////

// Information about a class member.
template <class Member>
struct ClassMemberInfo;

template <class ClassType, class ReturnType, class... Args>
struct ClassMemberInfo<ReturnType ( ClassType::* )( Args... ) const> {
	typedef ReturnType ReturnType;
	static const int ArgCount = sizeof...( Args );
	typedef CTuple<Args...> ArgsTuple;
	template <int argNum>
	using ArgTypeAt = typename VarArgs::At<argNum, Args...>::Result;
};

// General information about an entity that defines operator().
template <class F>
struct FunctionInfo : public ClassMemberInfo<decltype( &Types::PureType<F>::Result::operator() )> {};

template <class ReturnType, class... Args>
struct FunctionInfo<ReturnType( Args... )> {
	typedef ReturnType ReturnType;
	static const int ArgCount = sizeof...( Args );
	template <int argNum>
	using ArgTypeAt = typename VarArgs::At<argNum, Args...>::Result;
	typedef CTuple<Args...> ArgsTuple;
};

template <class ReturnType, class... Args>
struct FunctionInfo<ReturnType(*)( Args... )> : public FunctionInfo<ReturnType( Args... )> {
};

template <class ClassType, class ReturnType, class... Args>
struct FunctionInfo<ReturnType ( ClassType::* )( Args... ) const> {
	typedef ReturnType ReturnType;
	static const int ArgCount = sizeof...( Args ) + 1;
	typedef CTuple<const ClassType&, Args...> ArgsTuple;
	template <int argNum>
	using ArgTypeAt = typename VarArgs::At<argNum, const ClassType&, Args...>::Result;
};

// Class members can be also interpreted as functions taking an class object argument.
template <class ClassType, class ReturnType, class... Args>
struct FunctionInfo<ReturnType ( ClassType::* )( Args... )> {
	typedef ReturnType ReturnType;
	static const int ArgCount = sizeof...( Args ) + 1;
	typedef CTuple<ClassType&, Args...> ArgsTuple;
	template <int argNum>
	using ArgTypeAt = typename VarArgs::At<argNum, ClassType&, Args...>::Result;
};

}	// namespace Types.

//////////////////////////////////////////////////////////////////////////

// Class that represents a portion of source code to run. Can be a function or an object method.
template <class Callable>
class IAction;

template <class ReturnType, class... Args>
class IAction<ReturnType( Args... )> : public IExternalObject {
public:
	typedef ReturnType TReturnType;
	template <int argNum>
	using TArgType = typename VarArgs::At<argNum, Args...>::Result;

	virtual ReturnType Invoke( Args... args ) const = 0;
	ReturnType operator()( Args... args ) const
		{ return Invoke( forward<Args>( args )... ); }
};

//////////////////////////////////////////////////////////////////////////

// Action interface for self-changing actions.
template <class Callable>
class IMutableAction;

template <class ReturnType, class... Args>
class IMutableAction<ReturnType( Args... )> : public IExternalObject {
public:
	typedef ReturnType TReturnType;
	template <int argNum>
	using TArgType = typename VarArgs::At<argNum, Args...>::Result;

	virtual ReturnType Invoke( Args... args ) = 0;
	ReturnType operator()( Args... args )
		{ return Invoke( forward<Args>( args )... ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

