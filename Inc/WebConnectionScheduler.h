#pragma once
#include <Redefs.h>
#include <ReadWriteLock.h>
#include <Array.h>
#include <InternetFile.h>
#include <InternetFileBatch.h>
#include <Future.h>
#include <Promise.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

class REAPI CWebConnectionScheduler {
public:
	CWebConnectionScheduler() = default;
	~CWebConnectionScheduler();

	CFuture<CArray<BYTE>> ScheduleDownload( CInternetFile connection );
	CFuture<CArray<BYTE>> ScheduleUpload( CInternetFile connection, CArray<BYTE> data );

	// Wait for upcoming connection changes and perform them.
	void Run( int pollTimeoutMs );
	// Wake up the thread that is currently waiting inside the Run call.
	void WakeUp();

private:
	CInternetFileBatch batchConnection;

	struct CWebConnection {
		CInternetFile Connection;
		CPromise<CArray<BYTE>> Promise;
		CArray<BYTE> UploadData;
		CPtrOwner<CArray<BYTE>> DownloadData;

		explicit CWebConnection( CInternetFile connection ) : Connection( move( connection ) ) { DownloadData = CreateOwner<CArray<BYTE>>(); }
	};

	CReadWriteSection pendingSection;
	CArray<CWebConnection> pendingConnections;
	CArray<CWebConnection> activeConnections;

	void addPendingConnections();
	void detachConnection( CCurlEasyHandle handle );
	void attachConnection( CWebConnection& connection );
	int findConnectionIndex( CCurlEasyHandle handle );

	// Copying is prohibited.
	CWebConnectionScheduler( const CWebConnectionScheduler& ) = delete;
	void operator=( const CWebConnectionScheduler& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.