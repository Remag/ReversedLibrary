#pragma once
#include <Redefs.h>
#include <ReadWriteLock.h>
#include <Optional.h>
#include <Reassert.h>
#include <Remath.h>
#include <Action.h>
#include <Ptr.h>
#include <StaticAllocators.h>

namespace Relib {

template <class T>
class CFuture;

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class T>
class CFutureSharedState {
public:
	CFutureSharedState();

	// Mark the future as the one that will never be completed.
	void Abandon();

	T& WaitForValue();
	const T& WaitForValue() const;

	T* TryGetValue();
	const T* TryGetValue() const;

	template <class... Args>
	void CreateValue( Args&&... createArgs );

	template <class Func>
	auto AttachContinuation( Func&& action );

private:
	CReadWriteSection valueSection;
	COptional<CWriteLock> sectionLock;
	COptional<T> value;
	CReadWriteSection continuationSection;
	CActionOwner<void( T& )> continuationAction;
	CActionOwner<void()> abandonAction;
	bool isAbandoned = false;

	template <class Func>
	auto attachContinuation( Func&& action, Types::FalseType voidReturnMark );
	template <class Func>
	void attachContinuation( Func&& action, Types::TrueType voidReturnMark );
};

//////////////////////////////////////////////////////////////////////////

template<class T>
inline CFutureSharedState<T>::CFutureSharedState() :
	sectionLock( valueSection )
{
}

template<class T>
inline void CFutureSharedState<T>::Abandon()
{
	CWriteLock lock( continuationSection );
	continuationAction = CActionOwner<void( T& )>();
	if( !abandonAction.IsNull() ) {
		abandonAction();
		abandonAction = CActionOwner<void()>();
	}
	isAbandoned = true;
}

template<class T>
inline T& CFutureSharedState<T>::WaitForValue()
{
	CReadLock lock( valueSection );
	assert( value.IsValid() );
	return *value;
}

template<class T>
inline const T& CFutureSharedState<T>::WaitForValue() const
{
	CReadLock lock( valueSection );
	assert( value.IsValid() );
	return *value;
}

template<class T>
inline T* CFutureSharedState<T>::TryGetValue()
{
	auto lock = valueSection.TryLockRead();
	return lock.IsValid() ? &( *value ) : nullptr;
}

template<class T>
inline const T* CFutureSharedState<T>::TryGetValue() const
{
	auto lock = valueSection.TryLockRead();
	return lock.IsValid() ? &( *value ) : nullptr;
}

template<class T>
template<class... Args>
inline void CFutureSharedState<T>::CreateValue( Args&&... createArgs )
{
	CWriteLock lock( continuationSection );
	assert( !isAbandoned && !value.IsValid() );
	value.CreateValue( forward<Args>( createArgs )... );
	sectionLock.DeleteValue();
	
	if( !continuationAction.IsNull() ) {
		continuationAction( *value );
		continuationAction = CActionOwner<void( T& )>{};
		abandonAction = CActionOwner<void()>();
	}
}

template<class T>
template<class Func>
inline auto CFutureSharedState<T>::AttachContinuation( Func&& action )
{
	CWriteLock lock( continuationSection );
	typedef typename Types::FunctionInfo<Func>::ReturnType TReturnType;
	return attachContinuation( forward<Func>( action ), Types::IsSame<TReturnType, void>() );
}

template<class T>
template<class Func>
inline auto CFutureSharedState<T>::attachContinuation( Func&& action, Types::FalseType /*voidReturnMark*/ )
{
	typedef typename Types::FunctionInfo<Func>::ReturnType TReturnType;
	auto continuationState = CreateShared<CFutureSharedState<TReturnType>, CProcessHeap>();
	auto newAction = [prevAction = move( continuationAction ), state = continuationState, userAction = forward<Func>( action )]( T& arg ) {
		if( !prevAction.IsNull() ) {
			prevAction( arg );
		}
		state->CreateValue( userAction( arg ) );
	};
	continuationAction = move( newAction );
	// Attach an action that recursively abandons continuations.
	auto newAbandonAction = [prevAction = move( abandonAction ), state = continuationState]() {
		if( !prevAction.IsNull() ) {
			prevAction();
		}
		state->Abandon();
	};
	abandonAction = move( newAbandonAction );
	if( value.IsValid() ) {
		// Executed if the value is already created.
		continuationAction( *value );
		continuationAction = CActionOwner<void( T& )>{};
		abandonAction = CActionOwner<void()>();
	}
	return CFuture<TReturnType>( move( continuationState ) );
}

template<class T>
template<class Func>
inline void CFutureSharedState<T>::attachContinuation( Func&& action, Types::TrueType /*voidReturnMark*/ )
{
	auto newAction = [prevAction = move( continuationAction ), userAction = forward<Func>( action )]( T& arg ) {
		if( !prevAction.IsNull() ) {
			prevAction( arg );
		}
		userAction( arg );
	};
	continuationAction = move( newAction );
	if( value.IsValid() ) {
		// Executed if the value is already created.
		continuationAction( *value );
		continuationAction = CActionOwner<void( T& )>{};
		abandonAction = CActionOwner<void()>();
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.
