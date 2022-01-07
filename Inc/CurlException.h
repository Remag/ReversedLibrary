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
	explicit CCurlException( CUnicodeString msg ) : errorMsg( move( msg ) ) {}

	virtual CUnicodeString GetMessageText() const override final;

private:
	CUnicodeString errorMsg;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET

