#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_INTERNET
#include <CurlEasyHandle.h>

namespace Relib {

typedef void CURLM;
//////////////////////////////////////////////////////////////////////////

struct CEasyHandleResultData {
	CCurlEasyHandle Handle;
	int ErrorCode = NotFound;
};

//////////////////////////////////////////////////////////////////////////

// A range of CURL handles with finished transfers.
// Provides minimal range-based for loop support.
class REAPI CCompletedHandlesRange {
public:
	explicit CCompletedHandlesRange( CURLM* _handle ) : multiHandle( _handle ), currentEasyHandle( nullptr ) { advanceReadMessage(); }

	void operator++()
		{ advanceReadMessage(); }
	CEasyHandleResultData operator*() const
		{ return CEasyHandleResultData{ currentEasyHandle, currentErrorCode }; }
	bool operator!=( const CCompletedHandlesRange& other ) const
		{ return multiHandle != other.multiHandle; }

	CCompletedHandlesRange begin() const
		{ return *this; }
	CCompletedHandlesRange end() const
		{ return CCompletedHandlesRange( nullptr ); }

private:
	CURLM* multiHandle;
	CCurlEasyHandle currentEasyHandle;
	int currentErrorCode = NotFound;

	void advanceReadMessage();
};

//////////////////////////////////////////////////////////////////////////

class CInternetFileBatch {
public:
	CInternetFileBatch();
	// Destroy the file batch. All attached files need to be detached.
	~CInternetFileBatch();

	// Attach a download operation with a given handle.
	void AttachDownload( CCurlEasyHandle connection, CArray<BYTE>& downloadBuffer );
	// Attach an upload with a given handle. Upload data must not be destroyed until the handle is detached
	void AttachUpload( CCurlEasyHandle connection, CArrayView<BYTE> data, CArray<BYTE>& downloadBuffer );

	// Detach a given connection before it's finished. 
	void DetachConnection( CCurlEasyHandle connection );
	
	// Wait for the attached handles to have something interesting to do.
	void Poll( int timeoutMs );
	// Wakeup the thread that is currently polling on this handle.
	void WakeUp();
	// Perform all currently ready operations.
	CCompletedHandlesRange Perform();

private:
	CURLM* multiHandle = nullptr;

	void checkMultiCurlError( bool condition );
	void checkMultiCurlError( int errorCode );

	// Copying is prohibited.
	CInternetFileBatch( const CInternetFileBatch& ) = delete;
	void operator=( const CInternetFileBatch& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET