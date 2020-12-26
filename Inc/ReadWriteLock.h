#pragma once
#include <Redefs.h>
#include <Optional.h>
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

	COptional<CReadLock> TryLockRead() const;
	COptional<CWriteLock> TryLockWrite();

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
	CReadLock() = default;
	explicit CReadLock( const CReadWriteSection& target ) : lock( &target ) { lock->LockRead(); }
	CReadLock( CReadLock&& other ) : lock( other.lock ) { other.lock = nullptr; }
	CReadLock& operator=( CReadLock&& other );
	~CReadLock();

	const CReadWriteSection& GetSection() const
		{ return *lock; }

	// Sections can create locks without locking them.
	friend class CReadWriteSection;

private:
	const CReadWriteSection* lock = nullptr;

	CReadLock( const CReadWriteSection& target, bool /*noLock*/ ) : lock( &target ) {}

	// Copying is prohibited.
	CReadLock( const CReadLock& ) = delete;
	void operator=( const CReadLock& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

class CWriteLock {
public:
	CWriteLock() = default;
	explicit CWriteLock( CReadWriteSection& target ) : lock( &target ) { lock->LockWrite(); }
	CWriteLock( CWriteLock&& other ) : lock( other.lock ) { other.lock = nullptr; }
	CWriteLock& operator=( CWriteLock&& other );
	~CWriteLock();

	CReadWriteSection& GetSection()
		{ return *lock; }

	// Sections can create locks without locking them.
	friend class CReadWriteSection;

private:
	CReadWriteSection* lock = nullptr;

	CWriteLock( CReadWriteSection& target, bool /*noLock*/ ) : lock( &target ) {}

	// Copying is prohibited.
	CWriteLock( const CWriteLock& ) = delete;
	void operator=( const CWriteLock& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

inline COptional<CReadLock> CReadWriteSection::TryLockRead() const
{
	const auto result = ::TryAcquireSRWLockShared( &lock );
	return ( result != 0 ) ? CreateOptional( CReadLock( *this, false ) ) : COptional<CReadLock>();
}

inline COptional<CWriteLock> CReadWriteSection::TryLockWrite()
{
	const auto result = ::TryAcquireSRWLockExclusive( &lock );
	return ( result != 0 ) ? CreateOptional( CWriteLock( *this, false ) ) : COptional<CWriteLock>();
}

//////////////////////////////////////////////////////////////////////////

inline CReadLock& CReadLock::operator=( CReadLock&& other )
{
	swap( lock, other.lock );
	return *this;
}

inline CReadLock::~CReadLock()
{
	if( lock != nullptr ) {
		lock->UnlockRead();
	}
}

//////////////////////////////////////////////////////////////////////////

inline CWriteLock& CWriteLock::operator=( CWriteLock&& other )
{
	swap( lock, other.lock );
	return *this;
}

inline CWriteLock::~CWriteLock()
{
	if( lock != nullptr ) {
		lock->UnlockWrite();
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

