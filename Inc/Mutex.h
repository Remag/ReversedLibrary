#pragma once
#include <BaseStringView.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A wrapper around a WinApi mutex object. Used for synchronization between shared data in different processes.
class REAPI CMutex {
public:
	explicit CMutex( CUnicodeView mutexName )
		{ mutexHandle = ::CreateMutex( nullptr, FALSE, mutexName.Ptr() ); }
	~CMutex()
		{ ::CloseHandle( mutexHandle ); }

	void Lock() const
		{ ::WaitForSingleObject( mutexHandle, INFINITE ); }
	void Unlock() const
		{ ::ReleaseMutex( mutexHandle ); }

private:
	HANDLE mutexHandle;

	// Copying is prohibited.
	CMutex( CMutex& ) = delete;
	void operator=( CMutex& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// A helper class that uses RAII to lock a mutex.
class REAPI CMutexLock {
public:
	explicit CMutexLock( const CMutex& _mutex ) : mutex( _mutex ) { mutex.Lock(); }
	~CMutexLock()
		{ mutex.Unlock(); }

private:
	const CMutex& mutex;

	// Copying is prohibited.
	CMutexLock( CMutexLock& ) = delete;
	void operator=( CMutexLock& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

