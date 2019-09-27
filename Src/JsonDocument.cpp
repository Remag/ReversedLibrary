#include <JsonDocument.h>
#include <JsonValues.h>
#include <FileOwners.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const REAPI CUnicodeView JsonParsingError;
CUnicodeString CJsonParseException::GetMessageText() const
{
	return JsonParsingError.SubstParam( parsePos );
}

//////////////////////////////////////////////////////////////////////////

void CJsonDocument::Empty()
{
	root = nullptr;
	jsonData.Reset();
}

void CJsonDocument::CreateFromFile( CUnicodeView fileName )
{
	CFileReader jsonFile( fileName, FCM_OpenExisting );
	const auto length = jsonFile.GetLength32();
	auto jsonStrBuffer = jsonData.Create( length + 1, alignof( char ) );
	jsonFile.ReadExact( jsonStrBuffer.Ptr(), length );
	reinterpret_cast<BYTE*>( jsonStrBuffer.Ptr() )[length] = 0;
	const CStringView jsonStr( static_cast<const char*>( jsonStrBuffer.Ptr() ), length );
	parseJson( jsonStr );
}

void CJsonDocument::CreateFromString( CStringPart str )
{
	const auto jsonStr = allocateStringView( str );
	parseJson( jsonStr );
}

CString CJsonDocument::GetDocumentString() const
{
	assert( root != nullptr );
	CString result;
	writeToString( *root, result );
	return result;
}

void CJsonDocument::parseJson( CStringView jsonStr )
{
	int startPos = 0;
	root = &parseElement( jsonStr, startPos );
}

void CJsonDocument::throwParseException( int parsePos ) const
{
	throw CJsonParseException( parsePos );
}

int CJsonDocument::skipWhitespace( CStringView str, int startPos ) const
{
	int resultPos = startPos;
	while( CStringPart::IsCharWhiteSpace( str[resultPos] ) ) {
		resultPos++;
	}
	return resultPos;
}

CStringPart CJsonDocument::parseString( CStringView str, int& parsePos )
{
	const auto strStartPos = parsePos + 1;
	const auto endOrEscapePos = str.FindOneOf( CStringView( "\\\"" ), strStartPos );
	if( endOrEscapePos == NotFound ) {
		throwParseException( parsePos );
	}
	if( str[endOrEscapePos] == '\\' ) {
		return replaceEscapeSequences( str, endOrEscapePos, parsePos );
	} else {
		parsePos = endOrEscapePos + 1;
		return str.Mid( strStartPos, endOrEscapePos - strStartPos );
	}
}

CStringPart CJsonDocument::replaceEscapeSequences( CStringView str, int firstEscapePos, int& parsePos )
{
	const auto startPos = parsePos + 1;
	auto strPtr = const_cast<char*>( str.Ptr() );
	int destStrPos = firstEscapePos;
	int srcStrPos = firstEscapePos + 1;
	strPtr[destStrPos++] = getEscapeCharacter( str[srcStrPos++] );
	for( ;; ) {
		const auto ch = str[srcStrPos];
		if( ch == L'\\' ) {
			const auto escapeCh = getEscapeCharacter( str[srcStrPos + 1] );
			strPtr[destStrPos++] = escapeCh;
			srcStrPos += 2;
		} else if( ch == L'"' ) {
			parsePos = srcStrPos + 1;
			return str.Mid( startPos, destStrPos - startPos );
		} else if( ch == 0 ) {
			throwParseException( parsePos );
		} else {
			strPtr[destStrPos++] = str[srcStrPos++];
		}
	}
}

char CJsonDocument::getEscapeCharacter( char escapeCode ) const
{
	switch( escapeCode ) {
		case '"':
		case '\\':
		case '/':
			return escapeCode;
		case 'b':
			return '\b';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		default:
			return escapeCode;
	}
}

double CJsonDocument::parseNumber( CStringView str, int& parsePos ) const
{
	int endPos = parsePos;
	while( CStringView::IsCharDigit( str[endPos] ) ) {
		endPos++;
	}

	const auto numberStr = str.Mid( parsePos, endPos - parsePos );
	const auto result = Value<double>( numberStr );
	if( !result.IsValid() ) {
		throwParseException( parsePos );
	}
	parsePos = endPos;
	return *result;
}

CJsonObject& CJsonDocument::parseObject( CStringView str, int& parsePos )
{
	auto currentPos = skipWhitespace( str, parsePos + 1 );
	if( str[currentPos] == L'}' ) {
		parsePos = currentPos + 1;
		return allocateJsonObject( nullptr, nullptr, 0 );
	}

	auto currentNode = allocateListNode<CJsonKeyValue>();
	const auto listHead = currentNode;
	int listSize = 1;
	for( ;; ) {
		if( str[currentPos] != '"' ) {
			throwParseException( currentPos );
		}
		const auto keyName = parseString( str, currentPos );
		currentPos = skipWhitespace( str, currentPos );
		if( str[currentPos] != ':' ) {
			throwParseException( currentPos );
		}
		currentPos = skipWhitespace( str, currentPos + 1 );
		auto& keyValue = parseValue( str, currentPos );
		currentPos = skipWhitespace( str, currentPos );
		currentNode->Value = CJsonKeyValue{ keyName, &keyValue };
		if( str[currentPos] == L'}' ) {
			parsePos = currentPos + 1;
			return allocateJsonObject( listHead, currentNode, listSize );
		} else if( str[currentPos] != ',' ) {
			throwParseException( currentPos );
		}
		currentNode = addListNode( *currentNode );
		currentPos = skipWhitespace( str, currentPos + 1 );
		listSize++;
	}
}

CJsonValue& CJsonDocument::parseArray( CStringView str, int& parsePos )
{
	auto currentPos = skipWhitespace( str, parsePos + 1 );
	if( str[currentPos] == L']' ) {
		parsePos = currentPos + 1;
		return allocateJsonDynamicArray( nullptr, nullptr, 0 );
	}

	auto currentNode = allocateListNode<CJsonValue*>();
	const auto listHead = currentNode;
	int listSize = 1;
	for( ;; ) {
		auto& value = parseValue( str, currentPos );
		currentPos = skipWhitespace( str, currentPos );
		currentNode->Value = &value;
		if( str[currentPos] == L']' ) {
			parsePos = currentPos + 1;
			return allocateJsonDynamicArray( listHead, currentNode, listSize );
		} else if( str[currentPos] != ',' ) {
			throwParseException( currentPos );
		}
		currentNode = addListNode( *currentNode );
		currentPos = skipWhitespace( str, currentPos + 1 );
		listSize++;
	}
}

CJsonValue& CJsonDocument::parseValue( CStringView str, int& parsePos )
{
	const auto firstCh = str[parsePos];
	switch( firstCh ) {
		case 't':
			parsePos += 4;
			return allocateJsonBool( true );
		case 'f':
			parsePos += 5;
			return allocateJsonBool( false );
		case 'n':
			parsePos += 4;
			return allocateJsonNull();
		case '{':
			return parseObject( str, parsePos );
		case '[':
			return parseArray( str, parsePos );
		case '"':
			return allocateJsonString( parseString( str, parsePos ) );
		default:
			return allocateJsonNumber( parseNumber( str, parsePos ) );
	}
}

CJsonValue& CJsonDocument::parseElement( CStringView str, int& parsePos )
{
	parsePos = skipWhitespace( str, parsePos );
	auto& result = parseValue( str, parsePos );
	parsePos = skipWhitespace( str, parsePos );
	return result;
}

CStringView CJsonDocument::allocateStringView( CStringPart source )
{
	const auto length = source.Length();
	auto strBuffer = allocateStringBuffer( source, length + 1 );
	reinterpret_cast<BYTE*>( strBuffer.Ptr() )[length] = 0;
	return CStringView( static_cast<const char*>( strBuffer.Ptr() ), length );
}

CStringPart CJsonDocument::allocateStringPart( CStringPart source )
{
	const auto length = source.Length();
	const auto strBuffer = allocateStringBuffer( source, length );
	return CStringPart( static_cast<const char*>( strBuffer.Ptr() ), length );
}

CRawBuffer CJsonDocument::allocateStringBuffer( CStringPart source, int length )
{
	auto strBuffer = jsonData.Create( length, alignof( char ) );
	::memcpy( strBuffer.Ptr(), source.begin(), length );
	return strBuffer;
}

CJsonNull& CJsonDocument::allocateJsonNull()
{
	return jsonData.Create<CJsonNull>();
}

CJsonBool& CJsonDocument::allocateJsonBool( bool value )
{
	return jsonData.Create<CJsonBool>( value );
}

CJsonNumber& CJsonDocument::allocateJsonNumber( double value )
{
	return jsonData.Create<CJsonNumber>( value );
}

CJsonString& CJsonDocument::allocateJsonString( CStringPart value )
{
	return jsonData.Create<CJsonString>( value );
}

CJsonDynamicArray& CJsonDocument::allocateJsonDynamicArray( CJsonListNode<CJsonValue*>* head, CJsonListNode<CJsonValue*>* tail, int size )
{
	return jsonData.Create<CJsonDynamicArray>( head, tail, size );
}

CJsonObject& CJsonDocument::allocateJsonObject( CJsonListNode<CJsonKeyValue>* head, CJsonListNode<CJsonKeyValue>* tail, int size )
{
	return jsonData.Create<CJsonObject>( head, tail, size );
}

void CJsonDocument::writeToString( const CJsonValue& value, CString& result ) const
{
	staticAssert( JVT_EnumCount == 6 );
	switch( value.GetType() ) {
		case JVT_Null:
			writeToString( static_cast<const CJsonNull&>( value ), result );
			break;
		case JVT_Number:
			writeToString( static_cast<const CJsonNumber&>( value ), result );
			break;
		case JVT_String:
			writeToString( static_cast<const CJsonString&>( value ), result );
			break;
		case JVT_Bool:
			writeToString( static_cast<const CJsonBool&>( value ), result );
			break;
		case JVT_Array:
			writeToString( static_cast<const CJsonDynamicArray&>( value ), result );
			break;
		case JVT_Object:
			writeToString( static_cast<const CJsonObject&>( value ), result );
			break;
		default:
			assert( false );
			break;
	}
}

void CJsonDocument::writeToString( const CJsonNull&, CString& result ) const
{
	result += "null";
}

void CJsonDocument::writeToString( const CJsonNumber& value, CString& result ) const
{
	result += Str( value.GetNumber() );
}

void CJsonDocument::writeToString( const CJsonString& value, CString& result ) const
{
	writeStringValue( value.GetString(), result );
}

void CJsonDocument::writeStringValue( CStringPart str, CString & result ) const
{
	result.ReserveBuffer( result.Length() + str.Length() + 2 );

	result += '"';
	for( auto ch : str ) {
		switch( ch ) {
			case '"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '/':
				result += "\\/";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
			default:
				result += ch;
				break;
		}
	}
	result += '"';
}

void CJsonDocument::writeToString( const CJsonBool& value, CString& result ) const
{
	result += value.GetBool() ? "true" : "false";
}

void CJsonDocument::writeToString( const CJsonDynamicArray& arr, CString& result ) const
{
	const auto lastPos = arr.Size() - 1;
	int valuePos = 0;

	result += '[';
	for( const auto& value : arr.GetValues() ) {
		writeToString( *value, result );
		if( valuePos != lastPos ) {
			result += ',';
		}
		valuePos++;
	}
	result += ']';
}

void CJsonDocument::writeToString( const CJsonObject& value, CString& result ) const
{
	const auto lastPos = value.Size() - 1;
	int valuePos = 0;

	result += '{';
	for( const auto& keyValue : value.GetKeyValueList() ) {
		writeStringValue( keyValue.Key, result );
		result += ':';
		writeToString( *keyValue.Value, result );
		if( valuePos != lastPos ) {
			result += ',';
		}
		valuePos++;
	}
	result += '}';
}

CJsonNumber& CJsonDocument::CreateNumber( int value )
{
	return allocateJsonNumber( static_cast<double>( value ) );
}

CJsonNumber& CJsonDocument::CreateNumber( double value )
{
	return allocateJsonNumber( value );
}

CJsonString& CJsonDocument::CreateString( CStringPart str )
{
	const auto docStr = allocateStringPart( str );
	return allocateJsonString( docStr );
}

CJsonBool& CJsonDocument::CreateBool( bool value )
{
	return allocateJsonBool( value );
}

CJsonDynamicArray& CJsonDocument::CreateArray()
{
	return allocateJsonDynamicArray( nullptr, nullptr, 0 );
}

void CJsonDocument::AddValue( CJsonDynamicArray& arr, CJsonValue& val )
{
	const auto tail = arr.getTail();
	if( tail == nullptr ) {
		const auto newNode = allocateListNode<CJsonValue*>();
		newNode->Value = &val;
		arr.setFirstNode( newNode );
	} else {
		const auto newNode = addListNode( *tail );
		newNode->Value = &val;
		arr.setTail( newNode );
	}
}

CJsonObject& CJsonDocument::CreateObject()
{
	return allocateJsonObject( nullptr, nullptr, 0 );
}

void CJsonDocument::AddValue( CJsonObject& obj, CStringPart key, CJsonValue& val )
{
	const auto docKey = allocateStringPart( key );
	const auto tail = obj.getTail();
	if( tail == nullptr ) {
		const auto newNode = allocateListNode<CJsonKeyValue>();
		newNode->Value = CJsonKeyValue{ docKey, &val };
		obj.setFirstNode( newNode );
	} else {
		const auto newNode = addListNode( *tail );
		newNode->Value = CJsonKeyValue{ docKey, &val };
		obj.setTail( newNode );
	}
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.
