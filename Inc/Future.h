#pragma once
#include <FutureSharedState.h>
#include <Ptr.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class representing a value that is currently being created somewhere else.
template <class T> 
class CFuture {
public:
	// Futures are created by promises and continuations.
	explicit CFuture( CSharedPtr<RelibInternal::CFutureSharedState<T>, CProcessHeap> _sharedState ) : sharedState( move( _sharedState ) ) {}

	// Wait for the value to be created and retrieve it.
	T& GetValue();
	const T& GetValue() const;

	// Block until the value is created.
	void Wait() const;

	// Create a continuation that will be executed when the promise is fulfilled.
	// The action will be run on the same thread that created the value for the promise.
	// If the value has already been created, the action is run immediately.
	// Given action takes the created value as its argument and its return value is wrapped in a future and returned from this method.
	template <class Func>
	auto Then( Func&& action );

private:
	CSharedPtr<RelibInternal::CFutureSharedState<T>, CProcessHeap> sharedState;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
inline T& CFuture<T>::GetValue()
{
	return sharedState->WaitForValue();
}

template<class T>
inline const T& CFuture<T>::GetValue() const
{
	return sharedState->WaitForValue();
}

template<class T>
inline void CFuture<T>::Wait() const
{
	sharedState->WaitForValue();
}

template<class T>
template<class Func>
inline auto CFuture<T>::Then( Func&& action )
{
	staticAssert( Types::FunctionInfo<Func>::ArgCount == 1 );
	typedef typename Types::FunctionInfo<Func>::template ArgTypeAt<0> TFirstArg;
	return sharedState->AttachContinuation( forward<Func>( action ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.