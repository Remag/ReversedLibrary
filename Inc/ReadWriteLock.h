#pragma once
#include <Synchapi.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// An SRW lock wrapper.
class CReadWriteSection {
public:
	CReadWriteSection()
		{ ::InitializeSRWLock( &lock ); }

	SRWLOCK& Handle() const
		{ return lock; }

	// Locks access to writing.
	void LockRead() const
		{ ::AcquireSRWLockShared( &lock ); }
	void UnlockRead() const
		{ ::ReleaseSRWLockShared( &lock ); }

	// Locks access to reading and writing.
	void LockWrite()
		{ ::AcquireSRWLockExclusive( &lock ); }
	void UnlockWrite()
		{ ::ReleaseSRWLockExclusive( &lock ); }

private:
	// Read locking operations are logically constant.
	mutable SRWLOCK lock;

	// Copying is prohibited.
	CReadWriteSection( CReadWriteSection& ) = delete;
	void operator=( CReadWriteSection& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

class CReadLock {
public:
	explicit CReadLock( const CReadWriteSection& target ) : lock( target ) { lock.LockRead(); }
	~CReadLock()
		{ lock.UnlockRead(); }

	const CReadWriteSection& GetSection() const
		{ return lock; }

private:
	const CReadWriteSection& lock;
};

//////////////////////////////////////////////////////////////////////////

class CWriteLock {
public:
	explicit CWriteLock( CReadWriteSection& target ) : lock( target ) { lock.LockWrite(); }
	~CWriteLock()
		{ lock.UnlockWrite(); }

	CReadWriteSection& GetSection()
		{ return lock; }

private:
	CReadWriteSection& lock;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

