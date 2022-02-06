#include <Reassert.h>
#include <Errors.h>
#include <Reutils.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>

namespace Relib {

extern const CStringView DefaultAssertFailedMessage;
void GenerateInternalError( const char* functionName, const wchar_t* errorText, const wchar_t* fileName, int line )
{
	auto text = DefaultAssertFailedMessage.SubstParam( errorText, fileName, CStringView( functionName ), line );
	throw CInternalException( move( text ) );
}

}	// namespace Relib.