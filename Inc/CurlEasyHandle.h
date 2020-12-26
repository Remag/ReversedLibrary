#pragma once

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