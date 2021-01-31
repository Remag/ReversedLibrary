#include <InternetFileBatch.h>
#include <Array.h>
#include <CurlInitializer.h>
#include <CurlException.h>
#include <InternetFile.h>
#include <LibCurl\curl.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

void CCompletedHandlesRange::advanceReadMessage()
{
	// Loop through available messages in search for a completion message.
	for( ;; ) {
		int queueCount;
		const auto newMsgPtr = curl_multi_info_read( multiHandle, &queueCount );
		if( newMsgPtr == nullptr ) {
			multiHandle = nullptr;
			return;
		} else if( newMsgPtr->msg == CURLMSG_DONE ) {
			currentEasyHandle = CCurlEasyHandle( newMsgPtr->easy_handle );
			currentErrorCode = newMsgPtr->data.result;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

CInternetFileBatch::CInternetFileBatch()
{
	InitializeCurl();
	multiHandle = curl_multi_init();
	checkMultiCurlError( multiHandle != nullptr );
}

CInternetFileBatch::~CInternetFileBatch()
{
	curl_multi_cleanup( multiHandle );
}

void CInternetFileBatch::AttachDownload( CCurlEasyHandle file, CArray<BYTE>& downloadBuffer )
{
	const auto handle = file.GetHandle();
	CInternetFile::prepareGetRequest( handle );
	CInternetFile::setDownloadData( downloadBuffer, handle );
	const auto resultCode = curl_multi_add_handle( multiHandle, handle );
	checkMultiCurlError( resultCode );
}

void CInternetFileBatch::AttachUpload( CCurlEasyHandle file, CArrayView<BYTE> data, CArray<BYTE>& downloadBuffer )
{
	const auto handle = file.GetHandle();
	CInternetFile::preparePutRequest( handle, data );
	CInternetFile::setDownloadData( downloadBuffer, handle );
	const auto resultCode = curl_multi_add_handle( multiHandle, handle );
	checkMultiCurlError( resultCode );
}

void CInternetFileBatch::DetachConnection( CCurlEasyHandle connection )
{
	const auto resultCode = curl_multi_remove_handle( multiHandle, connection.GetHandle() );
	checkMultiCurlError( resultCode );
}

void CInternetFileBatch::Poll( int timeoutMs )
{
	const auto resultCode = curl_multi_poll( multiHandle, nullptr, 0, timeoutMs, nullptr );
	checkMultiCurlError( resultCode );
}

void CInternetFileBatch::WakeUp()
{
	const auto resultCode = curl_multi_wakeup( multiHandle );
	checkMultiCurlError( resultCode );
}

CCompletedHandlesRange CInternetFileBatch::Perform()
{
	int newActiveCount;
	const auto resultCode = curl_multi_perform( multiHandle, &newActiveCount );
	checkMultiCurlError( resultCode );
	return CCompletedHandlesRange( multiHandle );
}

extern const CUnicodeView GeneralMultiCurlError;
void CInternetFileBatch::checkMultiCurlError( bool condition )
{
	if( !condition ) {
		throw CCurlException( UnicodeStr( GeneralMultiCurlError ) );
	}
}

void CInternetFileBatch::checkMultiCurlError( int errorCode )
{
	if( errorCode != CURLM_OK ) {
		const auto strBuffer = curl_multi_strerror( static_cast<CURLMcode>( errorCode ) );
		throw CCurlException( UnicodeStr( strBuffer ) );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.