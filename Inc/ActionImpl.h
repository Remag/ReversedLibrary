#pragma once
#include <Action.h>
#include <Invoke.h>

namespace Relib {

template <class Callable>
class CAction;
//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

template <class CallableType, class ReturnType, class ArgTuple>
class CBaseAction;

template <class CallableType, class ReturnType, class... Args>
class CBaseAction<CallableType, ReturnType, CTuple<Args...>> : public IAction<ReturnType( Args... )> {
public:
	CBaseAction( CallableType _callable ) : callable( move( _callable ) ) {}

	typedef IAction<ReturnType( Args... )> TBaseActionType;
	
	virtual ReturnType Invoke( Args... args ) const override final
		{ return Relib::Invoke( callable, forward<Args>( args )... ); }

private:
	CallableType callable;
};

//////////////////////////////////////////////////////////////////////////

template <class CallableType, class ReturnType, class ArgTuple>
class CBaseMutableAction;

template <class CallableType, class ReturnType, class... Args>
class CBaseMutableAction<CallableType, ReturnType, CTuple<Args...>> : public IMutableAction<ReturnType( Args... )> {
public:
	CBaseMutableAction( CallableType _callable ) : callable( move( _callable ) ) {}

	typedef IMutableAction<ReturnType( Args... )> TBaseActionType;
	
	virtual ReturnType Invoke( Args... args ) override final
		{ return Relib::Invoke( callable, forward<Args>( args )... ); }

private:
	CallableType callable;
};

//////////////////////////////////////////////////////////////////////////

template <class CallableType>
auto doCreateAction( CallableType&& callable, Types::TrueType /*actionDerivativeMarker*/ )
{
	return forward<CallableType>( callable );
}

template <class CallableType>
auto doCreateAction( CallableType&& callable, Types::FalseType /*actionDerivativeMarker*/ )
{
	typedef Types::PureType<CallableType>::Result TPureCallableType;
	return CAction<TPureCallableType>( forward<CallableType>( callable ) );
}

//////////////////////////////////////////////////////////////////////////

} // namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// An action implementation.
template <class Callable>
class CAction : public RelibInternal::CBaseAction<Callable, typename Types::FunctionInfo<Callable>::ReturnType, typename Types::FunctionInfo<Callable>::ArgsTuple> {
	using RelibInternal::CBaseAction<Callable, typename Types::FunctionInfo<Callable>::ReturnType, typename Types::FunctionInfo<Callable>::ArgsTuple>::CBaseAction;
};

//////////////////////////////////////////////////////////////////////////

// Create an object that is derived from IAction.
// If callable is already an action derivative, it is simply moved as a return object.
// Otherwise a CAction wrapper is created.
template <class CallableType>
auto CreateAction( CallableType&& callable )
{
	typedef Types::PureType<CallableType>::Result TPureCallableType;
	typedef CAction<TPureCallableType>::TBaseActionType TBaseAction;
	return RelibInternal::doCreateAction( forward<CallableType>( callable ), Types::IsDerivedFrom<TPureCallableType, TBaseAction>() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

