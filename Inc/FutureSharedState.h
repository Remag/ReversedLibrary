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

	T& WaitForValue();
	const T& WaitForValue() const;

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
};

//////////////////////////////////////////////////////////////////////////

template<class T>
inline CFutureSharedState<T>::CFutureSharedState() :
	sectionLock( valueSection )
{
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
template<class... Args>
inline void CFutureSharedState<T>::CreateValue( Args&&... createArgs )
{
	CWriteLock lock( continuationSection );
	assert( !value.IsValid() );
	value.CreateValue( forward<Args>( createArgs )... );
	sectionLock.DeleteValue();
	
	continuationAction( *value );
	continuationAction = CActionOwner<void( T& )>{};
}

template<class T>
template<class Func>
inline auto CFutureSharedState<T>::AttachContinuation( Func&& action )
{
	CWriteLock lock( continuationSection );
	typedef typename Types::FunctionInfo<Func>::ReturnType TReturnType;
	auto continuationState = CreateShared<CFutureSharedState<TReturnType>, CProcessHeap>();
	auto newAction = [prevAction = move( continuationAction ), state = continuationState, userAction = forward<Func>( action )]( T& arg ) {
		if( !prevAction.IsNull() ) {
			prevAction( arg );
		}
		state->CreateValue( userAction( arg ) );
	};
	continuationAction = move( newAction );
	if( value.IsValid() ) {
		// Executed if the value is already created.
		continuationAction( *value );
		continuationAction = CActionOwner<void( T& )>{};
	}
	return CFuture<TReturnType>( move( continuationState ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.
