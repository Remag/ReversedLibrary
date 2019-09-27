#include <JsonValues.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const REAPI CUnicodeView JsonConversionError;
CJsonValueException::CJsonValueException( TJsonValueType expected, TJsonValueType actual )
{
	errorText = JsonConversionError.SubstParam( getValueTypeName( expected ), getValueTypeName( actual ) );
}

extern const REAPI CUnicodeView JsonMissingKeyError;
CJsonValueException::CJsonValueException( CUnicodePart missingKeyName )
{
	errorText = JsonMissingKeyError.SubstParam( missingKeyName );
}

CUnicodeView CJsonValueException::getValueTypeName( TJsonValueType type )
{
	staticAssert( JVT_EnumCount == 6 );
	switch( type ) {
		case JVT_Null:
			return L"null";
		case JVT_Bool:
			return L"boolean";
		case JVT_Number:
			return L"number";
		case JVT_String:
			return L"string";
		case JVT_Array:
			return L"array";
		case JVT_Object:
			return L"object";
		default:
			assert( false );
			return CUnicodeView();
	}
}

CUnicodeString CJsonValueException::GetMessageText() const
{
	return copy( errorText );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.
