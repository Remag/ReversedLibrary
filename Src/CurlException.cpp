#include <Redefs.h>

#ifndef RELIB_NO_INTERNET

#include <CurlException.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const CStringView GeneralCurlError;
CString CCurlException::GetMessageText() const
{
	return GeneralCurlError.SubstParam( errorMsg );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif	// RELIB_NO_INTERNET