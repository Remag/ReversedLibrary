#include <JsonDocument.h>
#include <JsonValues.h>
#include <FileOwners.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const REAPI CUnicodeView JsonParsingError;
CUnicodeString CJsonParseException::GetMessageText() const
{
	return JsonParsingError.SubstParam( lineNumber, linePos );
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
	CreateFromFile( jsonFile );
}

void CJsonDocument::CreateFromFile( CFileReader& jsonFile )
{
	Empty();
	const auto length = jsonFile.GetLength32();
	if( length == 0 ) {
		return;
	}
	auto jsonStrBuffer = jsonData.Create( length + 1, alignof( char ) );
	CArrayBuffer<BYTE> jsonArrayBuffer( static_cast<BYTE*>( jsonStrBuffer.Ptr() ), length + 1 );
	int bufferStartPos;
	const auto encoding = jsonFile.ReadByteString( jsonArrayBuffer.Left( length ), bufferStartPos );
	assert( encoding == FTE_UTF8 || encoding == FTE_Undefined );
	jsonArrayBuffer[length] = 0;
	const CStringView jsonStr( reinterpret_cast<const char*>( jsonArrayBuffer.Mid( bufferStartPos ).Ptr() ), length - bufferStartPos );
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
	CJsonPosition startPos;
	auto& parseResult = parseElement( jsonStr, startPos );
	if( startPos.Pos != jsonStr.Length() ) {
		throwParseException( startPos );
	}
	root = &parseResult;
}

void CJsonDocument::throwParseException( CJsonPosition parsePos ) const
{
	throw CJsonParseException( parsePos.LineNumber, parsePos.Pos - parsePos.LineStartPos );
}

CJsonDocument::CJsonPosition CJsonDocument::getNextPos( CJsonPosition parsePos ) const
{
	return CJsonPosition{ parsePos.Pos + 1, parsePos.LineNumber, parsePos.LineStartPos };
}

CJsonDocument::CJsonPosition CJsonDocument::skipWhitespace( CStringView str, CJsonPosition startPos ) const
{
	int resultPos = startPos.Pos;
	int lineNumber = startPos.LineNumber;
	int lineStartPos = startPos.LineStartPos;
	while( CStringPart::IsCharWhiteSpace( str[resultPos] ) ) {
		if( str[resultPos] == '\n' ) {
			lineNumber++;
			lineStartPos = resultPos + 1;
		}
		resultPos++;
	}
	return CJsonPosition{ resultPos, lineNumber, lineStartPos };
}

CStringPart CJsonDocument::parseString( CStringView str, CJsonPosition& parsePos )
{
	const auto strStartPos = parsePos.Pos + 1;
	const auto endOrEscapePos = str.FindOneOf( CStringView( "\\\"" ), strStartPos );
	if( endOrEscapePos == NotFound ) {
		throwParseException( parsePos );
	}
	if( str[endOrEscapePos] == '\\' ) {
		return replaceEscapeSequences( str, endOrEscapePos, parsePos );
	} else {
		parsePos.Pos = endOrEscapePos + 1;
		return str.Mid( strStartPos, endOrEscapePos - strStartPos );
	}
}

CStringPart CJsonDocument::replaceEscapeSequences( CStringView str, int firstEscapePos, CJsonPosition& parsePos )
{
	const auto startPos = parsePos.Pos + 1;
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
			parsePos.Pos = srcStrPos + 1;
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

double CJsonDocument::parseNumber( CStringView str, CJsonPosition& parsePos ) const
{
	const auto startPos = parsePos.Pos;
	int endPos = startPos;
	while( CStringView::IsCharDigit( str[endPos] ) || str[endPos] == '.' || str[endPos] == 'e' ) {
		endPos++;
	}

	const auto numberStr = str.Mid( startPos, endPos - startPos );
	const auto result = Value<double>( numberStr );
	if( !result.IsValid() ) {
		throwParseException( parsePos );
	}
	parsePos.Pos = endPos;
	return *result;
}

CJsonObject& CJsonDocument::parseObject( CStringView str, CJsonPosition& parsePos )
{
	const auto internalStartPos = getNextPos( parsePos );
	auto currentPos = skipWhitespace( str, internalStartPos );
	if( str[currentPos.Pos] == L'}' ) {
		parsePos = getNextPos( currentPos );
		return allocateJsonObject( nullptr, nullptr, 0 );
	}

	auto currentNode = allocateListNode<CJsonKeyValue>();
	const auto listHead = currentNode;
	int listSize = 1;
	for( ;; ) {
		if( str[currentPos.Pos] != '"' ) {
			throwParseException( currentPos );
		}
		const auto keyName = parseString( str, currentPos );
		currentPos = skipWhitespace( str, currentPos );
		if( str[currentPos.Pos] != ':' ) {
			throwParseException( currentPos );
		}
		currentPos = skipWhitespace( str, getNextPos( currentPos ) );
		auto& keyValue = parseValue( str, currentPos );
		currentPos = skipWhitespace( str, currentPos );
		currentNode->Value = CJsonKeyValue{ keyName, &keyValue };
		if( str[currentPos.Pos] == L'}' ) {
			parsePos = getNextPos( currentPos );
			return allocateJsonObject( listHead, currentNode, listSize );
		} else if( str[currentPos.Pos] != ',' ) {
			throwParseException( currentPos );
		}
		currentNode = addListNode( *currentNode );
		currentPos = skipWhitespace( str, getNextPos( currentPos ) );
		listSize++;
	}
}

CJsonValue& CJsonDocument::parseArray( CStringView str, CJsonPosition& parsePos )
{
	auto currentPos = skipWhitespace( str, getNextPos( parsePos ) );
	if( str[currentPos.Pos] == ']' ) {
		parsePos = getNextPos( currentPos );
		return allocateJsonDynamicArray( nullptr, nullptr, 0 );
	}

	auto& firstValue = parseValue( str, currentPos );
	parsePos = skipWhitespace( str, currentPos );
	return parseValueList( str, parsePos, ']', firstValue );
}

CJsonValue& CJsonDocument::parseValueList( CStringView str, CJsonPosition& currentPos, char terminator, CJsonValue& firstValue )
{
	auto currentNode = allocateListNode<CJsonValue*>();
	currentNode->Value = &firstValue;
	const auto listHead = currentNode;
	int listSize = 1;
	for( ;; ) {
		if( str[currentPos.Pos] == terminator ) {
			currentPos = getNextPos( currentPos );
			return allocateJsonDynamicArray( listHead, currentNode, listSize );
		} else if( str[currentPos.Pos] != ',' ) {
			throwParseException( currentPos );
		}
		currentPos = skipWhitespace( str, getNextPos( currentPos ) );
		currentNode = addListNode( *currentNode );
		auto& value = parseValue( str, currentPos );
		currentNode->Value = &value;
		currentPos = skipWhitespace( str, currentPos );
		listSize++;
	}
}

CJsonValue& CJsonDocument::parseValue( CStringView str, CJsonPosition& parsePos )
{
	const auto firstCh = str[parsePos.Pos];
	switch( firstCh ) {
		case 't':
			parsePos.Pos += 4;
			return allocateJsonBool( true );
		case 'f':
			parsePos.Pos += 5;
			return allocateJsonBool( false );
		case 'n':
			parsePos.Pos += 4;
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

CJsonValue& CJsonDocument::parseElement( CStringView str, CJsonPosition& parsePos )
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

void CJsonDocument::AddArrayValue( CJsonDynamicArray& arr, CJsonValue& val )
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

void CJsonDocument::AddObjectValue( CJsonObject& obj, CStringPart key, CJsonValue& val )
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
