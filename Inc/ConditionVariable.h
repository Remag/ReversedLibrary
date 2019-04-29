#pragma once
#include <Synchapi.h>
#include <CriticalSection.h>
#include <ReadWriteLock.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// CONDITION_VARIABLE wrapper.
class CConditionVariable {
public:
	CConditionVariable()
		{ ::InitializeConditionVariable( &cond ); }

	// Sleep on this variable. The lock is released during the sleep and is reacquired before the thread wakes up.
	// Since the sleeping threads can wake up spuriously, an additional predicate is passed.
	// The predicate is checked before going to sleep and after every wakeup. The function doesn't return until the predicate is true.
	template <class Predicate>
	void Sleep( CCriticalSectionLock& lock, Predicate wakeCondition ) const;
	template <class Predicate>
	void Sleep( CReadLock& lock, Predicate wakeCondition ) const;
	template <class Predicate>
	void Sleep( CWriteLock& lock, Predicate wakeCondition ) const;

	// Sleep on a lock for the specified time or until WakeOne is called from another thread.
	// Spurious wake ups are possible.
	void SleepFor( CCriticalSectionLock& lock, int sleepTime ) const;
	void SleepFor( CReadLock& lock, int sleepTime ) const;
	void SleepFor( CWriteLock& lock, int sleepTime ) const;

	// Wake threads that are sleeping on this variable.
	void WakeOne()
		{ ::WakeConditionVariable( &cond ); }
	void WakeAll()
		{ ::WakeAllConditionVariable( &cond ); }

private:
	// Sleep operation is logically constant, even though it changes the variable state.
	mutable CONDITION_VARIABLE cond;
};

//////////////////////////////////////////////////////////////////////////

template <class Predicate>
void CConditionVariable::Sleep( CCriticalSectionLock& lock, Predicate wakeCondition ) const
{
	while( !wakeCondition() ) {
		::SleepConditionVariableCS( &cond, &lock.GetSection().Handle(), INFINITE );
	}
}

template <class Predicate>
void CConditionVariable::Sleep( CReadLock& lock, Predicate wakeCondition ) const
{
	while( !wakeCondition() ) {
		::SleepConditionVariableSRW( &cond, &lock.GetSection().Handle(), INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED );
	}
}

template <class Predicate>
void CConditionVariable::Sleep( CWriteLock& lock, Predicate wakeCondition ) const
{
	while( !wakeCondition() ) {
		::SleepConditionVariableSRW( &cond, &lock.GetSection().Handle(), INFINITE, ~CONDITION_VARIABLE_LOCKMODE_SHARED );
	}
}

inline void CConditionVariable::SleepFor( CCriticalSectionLock& lock, int sleepTime ) const
{
	assert( sleepTime >= 0 );
	if( sleepTime > 0 ) {
		::SleepConditionVariableCS( &cond, &lock.GetSection().Handle(), static_cast<ULONG>( sleepTime ) );
	}
}

inline void CConditionVariable::SleepFor( CReadLock& lock, int sleepTime ) const
{
	assert( sleepTime >= 0 );
	if( sleepTime > 0 ) {
		::SleepConditionVariableSRW( &cond, &lock.GetSection().Handle(), static_cast<ULONG>( sleepTime ), CONDITION_VARIABLE_LOCKMODE_SHARED );
	}
}

inline void CConditionVariable::SleepFor( CWriteLock& lock, int sleepTime ) const
{
	assert( sleepTime >= 0 );
	if( sleepTime > 0 ) {
		::SleepConditionVariableSRW( &cond, &lock.GetSection().Handle(), static_cast<ULONG>( sleepTime ), 0 );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

