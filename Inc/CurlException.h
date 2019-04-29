#pragma once
#include <BaseString.h>
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Exception type for LibCurl related errors.
class CCurlException : public CException {
public:
	explicit CCurlException( CUnicodeString msg ) : errorMsg( move( msg ) ) {}

	virtual CUnicodeString GetMessageText() const override final
		{ return copy( errorMsg ); }

private:
	CUnicodeString errorMsg;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

