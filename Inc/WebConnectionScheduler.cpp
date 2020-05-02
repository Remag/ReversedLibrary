#include <WebConnectionScheduler.h>
#include <LibCurl\curl.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CWebConnectionScheduler::~CWebConnectionScheduler()
{
	for( auto& connection : activeConnections ) {
		batchConnection.DetachConnection( connection.Connection );
	}
}

CFuture<CArray<BYTE>> CWebConnectionScheduler::ScheduleDownload( CInternetFile connection )
{
	CWriteLock lock( pendingSection );
	auto& newConnection = pendingConnections.Add( move( connection ) );
	WakeUp();
	return newConnection.Promise.GetFuture();
}

CFuture<CArray<BYTE>> CWebConnectionScheduler::ScheduleUpload( CInternetFile connection, CArray<BYTE> data )
{
	assert( !data.IsEmpty() );
	CWriteLock lock( pendingSection );
	auto& newConnection = pendingConnections.Add( move( connection ) );
	newConnection.UploadData = move( data );
	WakeUp();
	return newConnection.Promise.GetFuture();
}

void CWebConnectionScheduler::Run( int pollTimeoutMs )
{
	addPendingConnections();
	batchConnection.Poll( pollTimeoutMs );
	auto finishedRange = batchConnection.Perform();
	for( auto connection : finishedRange ) {
		if( connection.ErrorCode == CURLE_OK ) {
			detachConnection( connection.Handle );
		} else {
			batchConnection.DetachConnection( connection.Handle );
			const auto connectionIndex = findConnectionIndex( connection.Handle );
			assert( connectionIndex != NotFound );
			attachConnection( activeConnections[connectionIndex] );
		}
	}
}

void CWebConnectionScheduler::WakeUp()
{
	batchConnection.WakeUp();
}

void CWebConnectionScheduler::addPendingConnections()
{
	CWriteLock lock( pendingSection );
	for( auto& connection : pendingConnections ) {
		attachConnection( connection );
		activeConnections.Add( move( connection ) );
	}
	pendingConnections.Empty();
}

void CWebConnectionScheduler::detachConnection( CCurlEasyHandle handle )
{
	batchConnection.DetachConnection( handle );
	const auto connectionIndex = findConnectionIndex( handle );
	assert( connectionIndex != NotFound );
	activeConnections[connectionIndex].Promise.CreateValue( move( *activeConnections[connectionIndex].DownloadData ) );
	activeConnections.DeleteAt( connectionIndex );
}

void CWebConnectionScheduler::attachConnection( CWebConnection& connection )
{
	if( connection.UploadData.IsEmpty() ) {
		batchConnection.AttachDownload( connection.Connection, *connection.DownloadData );
	} else {
		batchConnection.AttachUpload( connection.Connection, connection.UploadData, *connection.DownloadData );
	}
}

int CWebConnectionScheduler::findConnectionIndex( CCurlEasyHandle handle )
{
	for( int i = activeConnections.Size() - 1; i >= 0; i-- ) {
		const auto& connection = activeConnections[i].Connection;
		if( static_cast<CCurlEasyHandle>( connection ).GetHandle() == handle.GetHandle() ) {
			return i;
		}
	}
	return NotFound;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.