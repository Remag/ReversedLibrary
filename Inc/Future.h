#pragma once
#include <FutureSharedState.h>
#include <Ptr.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class representing a value that is currently being created somewhere else.
template <class T> 
class CFuture {
public:
	typedef T Elem;

	CFuture() = default;
	// Futures are created by promises and continuations.
	explicit CFuture( CSharedPtr<RelibInternal::CFutureSharedState<T>, CProcessHeap> _sharedState ) : sharedState( move( _sharedState ) ) {}
	// Unwrap a chain of futures.
	CFuture( CFuture<CFuture<T>> chainedFuture );

	bool IsNull() const
		{ return sharedState == nullptr; }
	void Release()
		{ sharedState = nullptr; }

	// Wait for the value to be created and retrieve it.
	T& GetValue();
	const T& GetValue() const;

	// Check for the value and retrieve it if its ready. Return null otherwise.
	T* TryGetValue();
	const T* TryGetValue() const;

	// Block until the value is created.
	void Wait() const;

	// Create a continuation that will be executed when the promise is fulfilled.
	// The action will be run on the same thread that created the value for the promise.
	// If the value has already been created, the action is run immediately.
	// Given action takes the created value as its argument and its return value is wrapped in a future and returned from this method.
	template <class Func>
	auto Then( Func&& action );

	// A promise needs access to shared state for comparison.
	friend class CPromise<T>;

private:
	CSharedPtr<RelibInternal::CFutureSharedState<T>, CProcessHeap> sharedState;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
inline CFuture<T>::CFuture( CFuture<CFuture<T>> chainedFuture )
{
	sharedState = CreateShared<RelibInternal::CFutureSharedState<T>, CProcessHeap>();
	auto firstContinuationHandler = [state = sharedState]( CFuture<T>& future ) {
		const auto secondContinuationHandler = [state]( const T& result ) {
			state->CreateValue( copy( result ) );
		};
		future.Then( secondContinuationHandler );
	};
	chainedFuture.Then( firstContinuationHandler );
}

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
inline T* CFuture<T>::TryGetValue()
{
	return sharedState->TryGetValue();
}

template<class T>
inline const T* CFuture<T>::TryGetValue() const
{
	return sharedState->TryGetValue();
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

namespace RelibInternal {

template <class FinalizerFunc, class FirstFuture>
inline void attachCounterContinuation( CSharedPtr<CAtomic<int>> counter, const FinalizerFunc& finalizer, FirstFuture& future )
{
	auto continuation = [atomicCounter = move( counter ), finalizer]( typename FirstFuture::Elem& ) {
		if( atomicCounter->PreDecrement() == 0 ) {
			finalizer();
		}
	};
	future.Then( move( continuation ) );
}

template <class FinalizerFunc, class FirstFuture, class... FutureList>
inline void attachCounterContinuation( CSharedPtr<CAtomic<int>> counter, const FinalizerFunc& finalizer, FirstFuture& firstFuture, FutureList&... rest )
{
	attachCounterContinuation( counter, finalizer, firstFuture );
	attachCounterContinuation( counter, finalizer, rest... );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Create a future that has been fulfilled.
template <class T>
CFuture<T> CreateReadyFuture( T&& futureValue )
{
	auto result = CreateShared<RelibInternal::CFutureSharedState<T>, CProcessHeap>();
	result->CreateValue( forward<T>( futureValue ) );
	return CFuture<T>( move( result ) );
}

//////////////////////////////////////////////////////////////////////////

// Invoke a given action after all the futures in the list are completed.
template<class Func, class... FutureList>
inline auto ExecuteAfterAll( Func&& action, FutureList&&... futures )
{
	typedef typename Types::FunctionInfo<Func>::ReturnType TReturnType;
	auto finalizationState = CreateShared<RelibInternal::CFutureSharedState<TReturnType>, CProcessHeap>();

	const auto futureCount = sizeof...( futures );
	auto counter = CreateShared<CAtomic<int>>( futureCount );

	auto finalizerFunction = [state = finalizationState, finalAction = forward<Func>( action ), futures...]() {
		state->CreateValue( finalAction( futures.GetValue()... ) );
	};
	RelibInternal::attachCounterContinuation( move( counter ), finalizerFunction, futures... );
	return CFuture<TReturnType>( move( finalizationState ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.