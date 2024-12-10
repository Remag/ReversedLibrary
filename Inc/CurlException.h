#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_INTERNET

#include <BaseString.h>
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Exception type for LibCurl related errors.
class CCurlException : public CException {
public:
	explicit CCurlException( CString msg ) : errorMsg( move( msg ) ) {}
	CCurlException( const CCurlException& other ) : errorMsg( copy( other.errorMsg ) ) {}

	virtual CString GetMessageText() const override final;

private:
	CString errorMsg;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET

