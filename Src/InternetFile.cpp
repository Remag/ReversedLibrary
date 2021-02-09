#include <InternetFile.h>
#include <BaseStringPart.h>
#include <StringData.h>
#include <StrConversions.h>
#include <BaseStringView.h>
#include <Array.h>
#include <CurlInitializer.h>
#include <CurlException.h>
#include <LibCurl\curl.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CInternetFile::CInternetFile()
{
	InitializeCurl();
	easyHandle = curl_easy_init();
	errorBuffer.ReserveBuffer( CURL_ERROR_SIZE );
	curl_easy_setopt( easyHandle, CURLOPT_WRITEFUNCTION, curlWriteFunction );
	curl_easy_setopt( easyHandle, CURLOPT_READFUNCTION, curlReadFunction );
	curl_easy_setopt( easyHandle, CURLOPT_ERRORBUFFER, errorBuffer.Ptr() );
	curl_easy_setopt( easyHandle, CURLOPT_XFERINFOFUNCTION, curlProgressCallback );
}

CInternetFile::CInternetFile( CStringView url ) :
	CInternetFile()
{
	curl_easy_setopt( easyHandle, CURLOPT_URL, url.Ptr() );
}

CInternetFile::CInternetFile( CInternetFile&& other ) :
	easyHandle( other.easyHandle ),
	errorBuffer( move( other.errorBuffer ) ),
	progressAction( move( other.progressAction ) ),
	headerList( other.headerList )
{
	other.easyHandle = nullptr;
	other.headerList = nullptr;
}

CInternetFile::~CInternetFile()
{
	if( easyHandle != nullptr ) {
		curl_slist_free_all( headerList );
		curl_easy_cleanup( easyHandle );
	}
}

CInternetFile& CInternetFile::operator=( CInternetFile&& other )
{
	swap( easyHandle, other.easyHandle );
	swap( errorBuffer, other.errorBuffer );
	swap( progressAction, other.progressAction );
	swap( headerList, other.headerList );
	return *this;
}

void CInternetFile::SetUrl( CStringView urlName )
{
	curl_easy_setopt( easyHandle, CURLOPT_URL, urlName.Ptr() );
}

void CInternetFile::setDownloadData( CArray<BYTE>& buffer, CURL* handle )
{
	curl_easy_setopt( handle, CURLOPT_WRITEDATA, &buffer );
}

void CInternetFile::prepareGetRequest( CURL* handle )
{
	curl_easy_setopt( handle, CURLOPT_CUSTOMREQUEST, nullptr );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDS, nullptr );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDSIZE, 0L );

	curl_easy_setopt( handle, CURLOPT_HTTPGET, 1L );
}

void CInternetFile::preparePutRequest( CURL* handle, CArrayView<BYTE> data )
{
	curl_easy_setopt( handle, CURLOPT_CUSTOMREQUEST, nullptr );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDS, nullptr );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDSIZE, 0L );

	curl_easy_setopt( handle, CURLOPT_INFILESIZE, numeric_cast<long>( data.Size() ) );
	curl_easy_setopt( handle, CURLOPT_UPLOAD, 1L );
}

void CInternetFile::preparePostRequest( CURL* handle, CArrayView<BYTE> data )
{
	curl_easy_setopt( handle, CURLOPT_CUSTOMREQUEST, nullptr );
	curl_easy_setopt( handle, CURLOPT_UPLOAD, 0L );
	curl_easy_setopt( handle, CURLOPT_POST, 1L );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDS, data.Ptr() );
	curl_easy_setopt( handle, CURLOPT_POSTFIELDSIZE, numeric_cast<long>( data.Size() ) );
}

void CInternetFile::preparePatchRequest( CURL* handle, CArrayView<BYTE> data )
{
	preparePostRequest( handle, data );
	curl_easy_setopt( handle, CURLOPT_CUSTOMREQUEST, "PATCH" );
}

void CInternetFile::DownloadFile( CArray<BYTE>& result )
{
	assert( result.IsEmpty() );
	prepareGetRequest( easyHandle );
	setDownloadData( result, easyHandle );

	const auto performResult = curl_easy_perform( easyHandle );
	checkCurlError( performResult == CURLE_OK );
}

void CInternetFile::UploadFile( CArrayView<BYTE> data, CArray<BYTE>& response )
{
	preparePutRequest( easyHandle, data );
	setDownloadData( response, easyHandle );

	CCurlReadData readData{ data, 0 };
	curl_easy_setopt( easyHandle, CURLOPT_READDATA, &readData );
	const auto performResult = curl_easy_perform( easyHandle );
	checkCurlError( performResult == CURLE_OK );
}

void CInternetFile::PostFile( CArrayView<BYTE> data, CArray<BYTE>& response )
{
	preparePostRequest( easyHandle, data );
	setDownloadData( response, easyHandle );
	const auto performResult = curl_easy_perform( easyHandle );
	checkCurlError( performResult == CURLE_OK );
}

void CInternetFile::PatchFile( CArrayView<BYTE> data, CArray<BYTE>& response )
{
	preparePatchRequest( easyHandle, data );
	setDownloadData( response, easyHandle );
	const auto performResult = curl_easy_perform( easyHandle );
	checkCurlError( performResult == CURLE_OK );
}

void CInternetFile::SetFollowRedirects( bool isSet )
{
	curl_easy_setopt( easyHandle, CURLOPT_FOLLOWLOCATION, isSet ? 1 : 0 );
}

void CInternetFile::EmptyCustomHeaders()
{
	curl_easy_setopt( easyHandle, CURLOPT_HTTPHEADER, nullptr );
	curl_slist_free_all( headerList );
	headerList = nullptr;
}

void CInternetFile::AddCustomHeader( CStringView headerStr )
{
	headerList = curl_slist_append( headerList, headerStr.Ptr() );
	curl_easy_setopt( easyHandle, CURLOPT_HTTPHEADER, headerList );
}

void CInternetFile::SetProgressFunction( TProgressAction newValue )
{
	progressAction = move( newValue );
	if( progressAction.IsNull() ) {
		curl_easy_setopt( easyHandle, CURLOPT_NOPROGRESS, 1 );
	} else {
		curl_easy_setopt( easyHandle, CURLOPT_XFERINFODATA, progressAction.GetAction() );
		curl_easy_setopt( easyHandle, CURLOPT_NOPROGRESS, 0 );
	}
}

int CInternetFile::curlProgressCallback( void* clientData, __int64 dlTotal, __int64 dlNow, __int64, __int64 )
{
	const auto action = static_cast<const IAction<bool( __int64, __int64 )>*>( clientData );
	assert( action != nullptr );
	return action->Invoke( dlNow, dlTotal ) ? 0 : 1;
}

size_t CInternetFile::curlWriteFunction( void* buffer, size_t size, size_t nmemb, void* userData )
{
	CArray<BYTE>& writeDest = *static_cast<CArray<BYTE>*>( userData );

	const auto byteSize = size * nmemb;
	const auto writePos = writeDest.Size();
	writeDest.IncreaseSize( writeDest.Size() + byteSize );
	::memcpy( writeDest.Ptr() + writePos, buffer, byteSize );
	return byteSize;
}

size_t CInternetFile::curlReadFunction( void* buffer, size_t size, size_t nmemb, void* userData )
{
	auto& readData = *static_cast<CCurlReadData*>( userData );
	const auto byteSize = static_cast<int>( size * nmemb );
	const auto sizeLeft = readData.Data.Size() - readData.DataPos;
	const auto writeSize = min( byteSize, sizeLeft );
	::memcpy( buffer, readData.Data.Ptr(), writeSize );
	readData.DataPos += writeSize;
	return writeSize;
}

void CInternetFile::checkCurlError( bool condition )
{
	if( !condition ) {
		auto errorStr = UnicodeStr( errorBuffer.Ptr() );
		throw CCurlException( move( errorStr ) );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
