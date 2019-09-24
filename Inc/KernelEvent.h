#pragma once
#include <BaseStringView.h>
#include <Errors.h>
#include <StackArray.h>
#include <ArrayBuffer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Event kernel object wrapper.
class CKernelEvent {
public:
	explicit CKernelEvent( CUnicodeView eventName, bool isManual );
	CKernelEvent( CKernelEvent&& other );
	CKernelEvent& operator=( CKernelEvent&& other );
	~CKernelEvent();

	HANDLE GetHandle() const
		{ return eventHandle; }

	void Signal();
	bool Wait();

	template <class... Events>
	static int WaitAny( const Events&... events );
	template <class... Events>
	static int WaitAnyTimeout( int msTimeout, const Events&... events );

private:
	HANDLE eventHandle;

	template <class... Events>
	static void fillEventHandles( CArrayBuffer<HANDLE> handles, const CKernelEvent& e, const Events&... rest );
	static void fillEventHandles( CArrayBuffer<HANDLE> handles, const CKernelEvent& e );

	template <class... Events>
	static int doWaitAny( DWORD timeout, const Events&... events );

	// Copying is prohibited.
	CKernelEvent( CKernelEvent& ) = delete;
	void operator=( CKernelEvent& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

inline CKernelEvent::CKernelEvent( CUnicodeView eventName, bool isManual )
{
	eventHandle = ::CreateEvent( nullptr, isManual ? TRUE : FALSE, FALSE, eventName.Ptr() );
	checkLastError( eventHandle != nullptr );
}

inline CKernelEvent::CKernelEvent( CKernelEvent&& other ) :
	eventHandle( other.eventHandle )
{
	other.eventHandle = nullptr;
}

inline CKernelEvent& CKernelEvent::operator=( CKernelEvent&& other )
{
	swap( eventHandle, other.eventHandle );
	return *this;
}

inline CKernelEvent::~CKernelEvent()
{
	if( eventHandle != nullptr ) {
		::CloseHandle( eventHandle );
	}
}

inline void CKernelEvent::Signal()
{
	::SetEvent( eventHandle );
}

inline bool CKernelEvent::Wait()
{
	return( ::WaitForSingleObject( eventHandle, INFINITE ) == WAIT_OBJECT_0 );
}

template<class... Events>
int CKernelEvent::WaitAny( const Events&... events )
{
	return doWaitAny( INFINITE, events... );
}

template<class... Events>
int CKernelEvent::WaitAnyTimeout( int msTimeout, const Events&... events )
{
	assert( msTimeout >= 0 );
	return doWaitAny( static_cast<DWORD>( msTimeout ), events... );
}

template<class... Events>
int CKernelEvent::doWaitAny( DWORD timeout, const Events&... events )
{
	static const int eventCount = sizeof...( Events );
	staticAssert( eventCount <= MAXIMUM_WAIT_OBJECTS );
	staticAssert( eventCount > 0 );
	CStackArray<HANDLE, eventCount> eventHandles;
	CKernelEvent::fillEventHandles( eventHandles, events... );
	const auto waitResult = ::WaitForMultipleObjects( eventCount, eventHandles.Ptr(), FALSE, timeout );
	checkLastError( waitResult != WAIT_FAILED );
	const auto eventId = waitResult - WAIT_OBJECT_0;
	return ( eventId >= 0 && eventId < eventCount ) ? eventId : NotFound;
}

template<class... Events>
void CKernelEvent::fillEventHandles( CArrayBuffer<HANDLE> handles, const CKernelEvent& e, const Events&... rest )
{
	handles[0] = e.GetHandle();
	fillEventHandles( handles.Mid( 1 ), rest... );
}

inline void CKernelEvent::fillEventHandles( CArrayBuffer<HANDLE> handles, const CKernelEvent& e )
{
	handles[0] = e.GetHandle();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
