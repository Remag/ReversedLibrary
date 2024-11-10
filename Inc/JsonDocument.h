#pragma once
#include <Arena.h>
#include <StringOperations.h>
#include <RawStringBuffer.h>
#include <BaseStringView.h>
#include <BaseStringPart.h> 
#include <StrConversions.h>
#include <BaseString.h>
#include <Errors.h>

namespace Relib {

class CJsonValue;
class CJsonNull;
class CJsonBool;
class CJsonNumber;
class CJsonString;
class CJsonDynamicArray;
class CJsonObject;
template <class T>
struct CJsonListNode;
struct CJsonKeyValue;
//////////////////////////////////////////////////////////////////////////

class CJsonParseException : public CException {
public:
	explicit CJsonParseException( int _lineNumber, int _linePos ) : lineNumber( _lineNumber ), linePos( _linePos ) {}

	// CException.
	virtual CString GetMessageText() const override;

private:
	int lineNumber;
	int linePos;
};

//////////////////////////////////////////////////////////////////////////

// Parsed JSON information.
class REAPI CJsonDocument {
public:
	CJsonDocument() = default;

	void Empty();
	bool IsEmpty() const
		{ return root == nullptr; }

	void CreateFromFile( CStringPart fileName );
	void CreateFromFile( CFileReader& fileReader );
	void CreateFromString( CStringPart str );

	CString GetDocumentString() const;
	CString GetFormattedString() const;

	CJsonValue& GetRoot()
		{ assert( root != nullptr ); return *root; }
	const CJsonValue& GetRoot() const
		{ assert( root != nullptr ); return *root; }
	CJsonValue* TryGetRoot()
		{ return root; }
	const CJsonValue* TryGetRoot() const
		{ return root; }
	// Set the specified value as document's root.
	// Value must have been created in this document.
	void SetRoot( CJsonValue& newRoot )
		{ root = &newRoot; }

	// Value creation functions.
	CJsonNumber& CreateNumber( int value );
	CJsonNumber& CreateNumber( double value );
	CJsonString& CreateString( CStringPart str );
	CJsonBool& CreateBool( bool value );

	CJsonDynamicArray& CreateArray();
	void AddArrayValue( CJsonDynamicArray& arr, CJsonValue& val );
	CJsonObject& CreateObject();
	void AddObjectValue( CJsonObject& obj, CStringPart key, CJsonValue& val );

private:
	CArena<> jsonData;
	CJsonValue* root = nullptr;

	struct CJsonPosition {
		int Pos = 0;
		int LineNumber = 1;
		int LineStartPos = 0;
	};

	void parseJson( CStringView jsonStr );

	CStringView allocateStringView( CStringPart source );
	CStringPart allocateStringPart( CStringPart source );
	CRawBuffer allocateStringBuffer( CStringPart source, int length );
	template <class T>
	CJsonListNode<T>* allocateListNode();
	template <class T>
	CJsonListNode<T>* addListNode( CJsonListNode<T>& currentNode );

	CJsonNull& allocateJsonNull();
	CJsonBool& allocateJsonBool( bool value );
	CJsonNumber& allocateJsonNumber( double value );
	CJsonString& allocateJsonString( CStringPart value );
	CJsonDynamicArray& allocateJsonDynamicArray( CJsonListNode<CJsonValue*>* head, CJsonListNode<CJsonValue*>* tail, int size );
	CJsonObject& allocateJsonObject( CJsonListNode<CJsonKeyValue>* head, CJsonListNode<CJsonKeyValue>* tail, int size );

	void throwParseException( CJsonPosition parsePos ) const;
	CJsonPosition getNextPos( CJsonPosition parsePos ) const;
	CJsonPosition skipWhitespace( CStringView str, CJsonPosition startPos ) const;
	CStringPart parseString( CStringView str, CJsonPosition& parsePos );
	double parseNumber( CStringView str, CJsonPosition& parsePos ) const;
	CJsonObject& parseObject( CStringView str, CJsonPosition& parsePos );
	CJsonValue& parseArray( CStringView str, CJsonPosition& parsePos );
	CJsonValue& parseValueList( CStringView str, CJsonPosition& parsePos, char terminator, CJsonValue& firstValue );
	CJsonValue& parseValue( CStringView str, CJsonPosition& parsePos );
	CJsonValue& parseElement( CStringView str, CJsonPosition& parsePos );

	CStringPart replaceEscapeSequences( CStringView str, int firstEscapePos, CJsonPosition& parsePos );
	char getEscapeCharacter( char escapeCode ) const;

	void writeToString( const CJsonValue& value, int indentValue, CString& result ) const;
	void writeToString( const CJsonNull& value, CString& result ) const;
	void writeToString( const CJsonNumber& value, CString& result ) const;
	void writeToString( const CJsonString& value, CString& result ) const;
	void writeStringValue( CStringPart str, CString& result ) const;
	void writeToString( const CJsonBool& value, CString& result ) const;
	void writeToString( const CJsonDynamicArray& value, int indentValue, CString& result ) const;
	void writeToString( const CJsonObject& value, int indentValue, CString& result ) const;

	void indentLine( int indentValue, CString& result ) const;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
CJsonListNode<T>* CJsonDocument::allocateListNode()
{
	auto& newNode = jsonData.Create<CJsonListNode<T>>();
	newNode.Next = nullptr;
	return &newNode;
}

template<class T>
CJsonListNode<T>* CJsonDocument::addListNode( CJsonListNode<T>& currentNode )
{
	auto& newNode = jsonData.Create<CJsonListNode<T>>();
	newNode.Next = nullptr;
	currentNode.Next = &newNode;
	return &newNode;
}

} // namespace Relib.

