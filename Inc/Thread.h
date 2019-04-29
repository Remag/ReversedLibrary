#pragma once
#include <Redefs.h>
#include <process.h>
#include <MutableActionOwner.h>
#include <MemoryOwner.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

enum TThreadWaitResult {
	TWR_Finished = WAIT_OBJECT_0,	// thread was finished.
	TWR_Timeout = WAIT_TIMEOUT,	// waiting timeout.
	TWR_Interrupted = WAIT_IO_COMPLETION,	// APC arrived at the callers queue.
};

//////////////////////////////////////////////////////////////////////////

// A thread wrapper.
class CThread {
public:
	// Construct thread that will be run later with a Start method.
	CThread() = default;

	// Construct and run the thread.
	template <class StartAction, class... StartArgs>
	explicit CThread( StartAction entryPoint, StartArgs... args );
	CThread( CThread&& other );
	CThread& operator=( CThread other );
	// Wait for the thread to finish and destroy the object afterwards.
	~CThread();

	HANDLE Handle() const
		{ return threadHandle; }
	DWORD ThreadId() const
		{ return threadId; }

	// Start a thread with a given function.
	template <class StartAction, class... StartArgs>
	void Start( StartAction entryPoint, StartArgs... args );
	// Wait for thread to finish.
	// Set alertable to true if wait can be interrupted by APC.
	TThreadWaitResult Wait( int time = INFINITE, bool alertable = false );
	// Get thread's exit code.
	DWORD ExitCode() const;

	// Forget the running thread.
	void Abandon();

private:
	// Thread's ID and Handle.
	DWORD threadId = 0;
	HANDLE threadHandle = nullptr;

	// Actual thread's entry point. Calls the user specified function with the passed parameter.
	static unsigned __stdcall threadProc( LPVOID parameter );

	// Copying is prohibited.
	CThread( const CThread& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class StartAction, class... StartArgs>
CThread::CThread( StartAction entryPoint, StartArgs... args )
{
	Start( move( entryPoint ), move( args )... );
}

inline CThread::CThread( CThread&& other ) :
	threadId( other.threadId ),
	threadHandle( other.threadHandle )
{
	other.Abandon();
}

inline CThread& CThread::operator=( CThread other )
{
	swap( threadId, other.threadId ),
	swap( threadHandle, other.threadHandle );
	return *this;
}

inline CThread::~CThread()
{
	if( threadHandle == nullptr ) {
		return;
	} 
	try {
		if( ::GetCurrentThreadId() != threadId ) {
			// Thread object should not be destroyed until the corresponding thread is finished.
			Wait();
		}
		checkLastError( ::CloseHandle( threadHandle ) != 0 );
		threadHandle = 0;
	} catch( const CException& e ) {
		Log::Exception( e );
		if( threadHandle != 0 ) {
			::CloseHandle( threadHandle );
		}
	}
}

template <class StartAction, class... StartArgs>
void CThread::Start( StartAction threadEntryPoint, StartArgs... args )
{
	assert( threadHandle == nullptr );

	// Allocate memory for the action object. It will later be passed to the thread.
	CMutableActionOwner<int()> entryPoint;
	auto entryPointOwnerPtr = RELIB_STATIC_ALLOCATE( CRuntimeHeap, sizeof( entryPoint ) );
	CMemoryOwner<CRuntimeHeap> entryPointOwner( entryPointOwnerPtr );
	::new( entryPointOwnerPtr ) CMutableActionOwner<int()>( [entryAction = move( threadEntryPoint ), argTuple = CTuple<StartArgs...>( move( args )... )]() mutable {
		return TupleInvoke( entryAction, move( argTuple ) ); 
	} );
	entryPointOwner.Detach();

	unsigned rawThreadId;
	const uintptr_t rawThreadHandle = _beginthreadex( 0, 0, threadProc, entryPointOwnerPtr, 0, &rawThreadId );
	staticAssert( sizeof( threadHandle ) == sizeof( rawThreadHandle ) );
	staticAssert( sizeof( threadId ) == sizeof( rawThreadId ) );
	threadHandle = reinterpret_cast<HANDLE>( rawThreadHandle );
	threadId = static_cast<DWORD>( rawThreadId );
}

inline TThreadWaitResult CThread::Wait( int time /*= INFINITE*/, bool alertable /*= false */ )
{
	const DWORD result = ::WaitForSingleObjectEx( threadHandle, time, numeric_cast<BOOL>( alertable ) );
	checkLastError( result != WAIT_FAILED );
	return TThreadWaitResult( result );
}

inline void CThread::Abandon()
{
	threadId = 0;
	threadHandle = nullptr;
}

inline DWORD CThread::ExitCode() const
{
	assert( threadHandle != 0 );
	DWORD result;
	checkLastError( ::GetExitCodeThread( threadHandle, &result ) != 0 );
	return result;
}

inline unsigned __stdcall CThread::threadProc( LPVOID parameter )
{
	CMemoryOwner<CRuntimeHeap> paramOwner( parameter );
	auto entryPointPtr = static_cast<CMutableActionOwner<int()>*>( parameter );
	unsigned exitCode;
	try {
		exitCode = entryPointPtr->Invoke();
	} catch( const CException& ) {
		exitCode = 0xFFFFFFFF;
	}
	
	entryPointPtr->~CMutableActionOwner();
	return exitCode;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

