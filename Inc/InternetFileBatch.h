#pragma once
#include <Redefs.h>
#include <CurlEasyHandle.h>

namespace Relib {

typedef void CURLM;
//////////////////////////////////////////////////////////////////////////

// A range of CURL handles with finished transfers.
// Provides minimal range-based for loop support.
class CCompletedHandlesRange {
public:
	explicit CCompletedHandlesRange( CURLM* _handle ) : multiHandle( _handle ), currentEasyHandle( nullptr ) { advanceReadMessage(); }

	void operator++()
		{ advanceReadMessage(); }
	CCurlEasyHandle operator*() const
		{ return currentEasyHandle; }
	bool operator!=( const CCompletedHandlesRange& other ) const
		{ return multiHandle != other.multiHandle; }

	CCompletedHandlesRange begin() const
		{ return *this; }
	CCompletedHandlesRange end() const
		{ return CCompletedHandlesRange( nullptr ); }

private:
	CURLM* multiHandle;
	CCurlEasyHandle currentEasyHandle;

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
	void Wakeup();
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