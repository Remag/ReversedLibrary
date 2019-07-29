#include <CurlException.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const CUnicodeView GeneralCurlError;
CUnicodeString CCurlException::GetMessageText() const
{
	return GeneralCurlError.SubstParam( errorMsg );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
