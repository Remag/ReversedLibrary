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
	explicit CJsonParseException( int _parsePos ) : parsePos( _parsePos ) {}

	// CException.
	virtual CUnicodeString GetMessageText() const override;

private:
	int parsePos;
};

//////////////////////////////////////////////////////////////////////////

// Parsed JSON information.
class REAPI CJsonDocument {
public:
	CJsonDocument() = default;

	void Empty();

	void CreateFromFile( CUnicodeView fileName );
	void CreateFromString( CStringPart str );

	CString GetDocumentString() const;

	const CJsonValue& GetRoot() const
		{ assert( root != nullptr ); return *root; }
	// Set the specified value as document's root.
	// Value should be created using this document.
	void SetRoot( const CJsonValue& newRoot )
		{ root = &newRoot; }

	// Value creation functions.
	CJsonNumber& CreateNumber( int value );
	CJsonNumber& CreateNumber( double value );
	CJsonString& CreateString( CStringPart str );
	CJsonBool& CreateBool( bool value );

	CJsonDynamicArray& CreateArray();
	void AddValue( CJsonDynamicArray& arr, CJsonValue& val );
	CJsonObject& CreateObject();
	void AddValue( CJsonObject& obj, CStringPart key, CJsonValue& val );

private:
	CArena<> jsonData;
	const CJsonValue* root = nullptr;

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

	void throwParseException( int parsePos ) const;
	int skipWhitespace( CStringView str, int startPos ) const;
	CStringPart parseString( CStringView str, int& parsePos );
	double parseNumber( CStringView str, int& parsePos ) const;
	CJsonObject& parseObject( CStringView str, int& parsePos );
	CJsonValue& parseArray( CStringView str, int& parsePos );
	CJsonValue& parseValue( CStringView str, int& parsePos );
	CJsonValue& parseElement( CStringView str, int& parsePos );

	CStringPart replaceEscapeSequences( CStringView str, int firstEscapePos, int& parsePos );
	char getEscapeCharacter( char escapeCode ) const;

	void writeToString( const CJsonValue& value, CString& result ) const;
	void writeToString( const CJsonNull& value, CString& result ) const;
	void writeToString( const CJsonNumber& value, CString& result ) const;
	void writeToString( const CJsonString& value, CString& result ) const;
	void writeStringValue( CStringPart str, CString& result ) const;
	void writeToString( const CJsonBool& value, CString& result ) const;
	void writeToString( const CJsonDynamicArray& value, CString& result ) const;
	void writeToString( const CJsonObject& value, CString& result ) const;

	CStringPart copyStringToDocument( CStringPart src );
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

