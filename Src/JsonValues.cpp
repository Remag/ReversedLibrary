#include <JsonValues.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const REAPI CStringView JsonConversionError;
CJsonValueException::CJsonValueException( TJsonValueType expected, TJsonValueType actual )
{
	errorText = JsonConversionError.SubstParam( getValueTypeName( expected ), getValueTypeName( actual ) );
}

extern const REAPI CStringView JsonMissingKeyError;
CJsonValueException::CJsonValueException( CStringPart missingKeyName )
{
	errorText = JsonMissingKeyError.SubstParam( missingKeyName );
}

CStringView CJsonValueException::getValueTypeName( TJsonValueType type )
{
	staticAssert( JVT_EnumCount == 6 );
	switch( type ) {
		case JVT_Null:
			return "null";
		case JVT_Bool:
			return "boolean";
		case JVT_Number:
			return "number";
		case JVT_String:
			return "string";
		case JVT_Array:
			return "array";
		case JVT_Object:
			return "object";
		default:
			assert( false );
			return CStringView();
	}
}

CString CJsonValueException::GetMessageText() const
{
	return copy( errorText );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.
