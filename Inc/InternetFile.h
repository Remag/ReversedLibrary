#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_INTERNET

#include <ActionOwner.h>
#include <Array.h>
#include <ArrayBuffer.h>
#include <CurlEasyHandle.h>

typedef void CURL;
struct curl_slist;
namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Remote file that is fetched from the given URL. LibCurl is used to establish the connection and download/upload the files.
class REAPI CInternetFile {
public:
	typedef CActionOwner<bool( __int64 current, __int64 total )> TProgressAction;

	CInternetFile();
	explicit CInternetFile( CStringView url );
	CInternetFile( CInternetFile&& other );
	~CInternetFile();

	CInternetFile& operator=( CInternetFile&& other );

	operator CCurlEasyHandle() const
		{ return CCurlEasyHandle( easyHandle ); }

	void SetUrl( CStringView urlName );

	void SetFollowRedirects( bool isSet );

	void EmptyCustomHeaders();
	void AddCustomHeader( CStringView headerStr );

	// Synchronous download and upload operations.
	// The thread will block until the operation is complete.
	void DownloadFile( CArray<BYTE>& result );
	// Upload a file using a PUT request.
	void UploadFile( CArrayView<BYTE> data, CArray<BYTE>& response );
	// Upload a file using a POST request.
	void PostFile( CArrayView<BYTE> data, CArray<BYTE>& response );
	// Upload a file using a PATCH request.
	void PatchFile( CArrayView<BYTE> data, CArray<BYTE>& response );
	// Delete a file using a DELETE request.
	void DeleteSrcFile( CArray<BYTE>& response );

	// Set an action that gets called when the download progress is made.
	// Function can return false if it wants to abort the download process, otherwise it should return true.
	// If an empty action is provided, the progress reporting is turned off.
	void SetProgressFunction( TProgressAction progressAction );

	// An internet batching mechanism needs access to setting upload parameters.
	friend class CInternetFileBatch;

private:
	CURL* easyHandle;
	CArray<char> errorBuffer;
	TProgressAction progressAction;
	curl_slist* headerList = nullptr;

	struct CCurlReadData {
		CArrayView<BYTE> Data;
		int DataPos;
	};

	static int curlProgressCallback( void* clientData, __int64 dlTotal, __int64 dlNow, __int64 ulTotal, __int64 ulNow );
	static size_t curlWriteFunction( void* buffer, size_t size, size_t nmemb, void* userData );
	static size_t curlReadFunction( void* buffer, size_t size, size_t nmemb, void* userData );
	void checkCurlError( bool condition );

	static void setDownloadData( CArray<BYTE>& buffer, CURL* easyHandle );
	
	static void prepareGetRequest( CURL* easyHandle );
	static void preparePutRequest( CURL* easyHandle, CArrayView<BYTE> data );
	static void preparePostRequest( CURL* easyHandle, CArrayView<BYTE> data );
	static void preparePatchRequest( CURL* easyHandle, CArrayView<BYTE> data );
	static void prepareDeleteRequest( CURL* easyHandle );

	// Copying is prohibited.
	CInternetFile( CInternetFile& ) = delete;
	void operator=( CInternetFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET

