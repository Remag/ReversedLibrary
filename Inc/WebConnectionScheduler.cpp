#include <WebConnectionScheduler.h>

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
	return newConnection.Promise.GetFuture();
}

CFuture<CArray<BYTE>> CWebConnectionScheduler::ScheduleUpload( CInternetFile connection, CArray<BYTE> data )
{
	assert( !data.IsEmpty() );
	CWriteLock lock( pendingSection );
	auto& newConnection = pendingConnections.Add( move( connection ) );
	newConnection.UploadData = move( data );
	return newConnection.Promise.GetFuture();
}

void CWebConnectionScheduler::Run( int pollTimeoutMs )
{
	addPendingConnections();
	batchConnection.Poll( pollTimeoutMs );
	auto finishedRange = batchConnection.Perform();
	for( auto connection : finishedRange ) {
		detachConnection( connection );
	}
}

void CWebConnectionScheduler::addPendingConnections()
{
	CWriteLock lock( pendingSection );
	for( auto& connection : pendingConnections ) {
		if( connection.UploadData.IsEmpty() ) {
			batchConnection.AttachDownload( connection.Connection, connection.DownloadData );
		} else {
			batchConnection.AttachUpload( connection.Connection, connection.UploadData, connection.DownloadData );
		}
		activeConnections.Add( move( connection ) );
	}
	pendingConnections.Empty();
}

void CWebConnectionScheduler::detachConnection( CCurlEasyHandle handle )
{
	batchConnection.DetachConnection( handle );
	for( int i = activeConnections.Size() - 1; i >= 0; i-- ) {
		const auto& connection = activeConnections[i].Connection;
		if( static_cast<CCurlEasyHandle>( connection ).GetHandle() == handle.GetHandle() ) {
			activeConnections[i].Promise.CreateValue( move( activeConnections[i].DownloadData ) );
			activeConnections.DeleteAt( i );
			return;
		}
	}
	assert( false );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.