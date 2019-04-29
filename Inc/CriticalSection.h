#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// High level critical section wrapper.
class REAPI CCriticalSection {
public:
	CCriticalSection()
		{ ::InitializeCriticalSection( &section ); }
	~CCriticalSection()
		{ ::DeleteCriticalSection( &section ); }

	CRITICAL_SECTION& Handle() const
		{ return section; }

	void Lock() const
		{ ::EnterCriticalSection( &section ); }
	void Unlock() const
		{ ::LeaveCriticalSection( &section ); }

private:
	// Waiting on a critical section is considered a constant operation.
	mutable CRITICAL_SECTION section;

	// Copying is prohibited.
	CCriticalSection( const CCriticalSection& ) = delete;
	void operator=( const CCriticalSection& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Switcher of the critical section lock.
class REAPI CCriticalSectionLock {
public:
	explicit CCriticalSectionLock( const CCriticalSection& _section );
	~CCriticalSectionLock();

	const CCriticalSection& GetSection() const
		{ return section; }

private:
	const CCriticalSection& section;

	// Copying is prohibited.
	CCriticalSectionLock( const CCriticalSectionLock& ) = delete;
	void operator=( const CCriticalSectionLock& ) = delete;
};

inline CCriticalSectionLock::CCriticalSectionLock( const CCriticalSection& _section ) :
	section( _section )
{
	section.Lock();
}

inline CCriticalSectionLock::~CCriticalSectionLock()
{
	section.Unlock();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

