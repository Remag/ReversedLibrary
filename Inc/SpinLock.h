#pragma once
#include <Atomic.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Simple lock based on an atomic boolean flag.
// Busy loops on the thread, until the lock can be acquired.
class CSpinLock {
public:
	explicit CSpinLock( CAtomic<bool>& _lock );
	~CSpinLock();

private:
	CAtomic<bool>& lock;
};

//////////////////////////////////////////////////////////////////////////

inline CSpinLock::CSpinLock( CAtomic<bool>& _lock ) :
	lock( _lock )
{
	bool unlockedValue = true;
	while( !lock.CompareExchangeWeak( unlockedValue, false ) ) {
		unlockedValue = true;
	}
}

inline CSpinLock::~CSpinLock()
{
	lock.Store( true );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

