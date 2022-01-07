#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_INTERNET

namespace Relib {

typedef void CURL;
//////////////////////////////////////////////////////////////////////////

// A thin wrapper around an easy LibCurl handle.
class CCurlEasyHandle {
public:
	explicit CCurlEasyHandle( CURL* _handle ) : handle( _handle ) {}

	CURL* GetHandle() const
		{ return handle; }

private:
	CURL* handle;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET