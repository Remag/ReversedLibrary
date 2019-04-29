#pragma once
#include <BaseStringView.h>
#include <Errors.h>

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
	void Wait();

private:
	HANDLE eventHandle;

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

inline void CKernelEvent::Wait()
{
	::WaitForSingleObject( eventHandle, INFINITE );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
