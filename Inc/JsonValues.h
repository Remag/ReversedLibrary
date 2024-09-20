#pragma once
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <RawStringBuffer.h>
#include <StrConversions.h>
#include <ArrayBuffer.h>
#include <Errors.h>

namespace Relib {

class CJsonValue;
//////////////////////////////////////////////////////////////////////////

// Possible JSON value type.
enum TJsonValueType {
	JVT_Null,
	JVT_Number,
	JVT_Bool,
	JVT_String,
	JVT_Array,
	JVT_Object,
	JVT_EnumCount
};

//////////////////////////////////////////////////////////////////////////

struct CJsonKeyValue {
	CStringPart Key;
	CJsonValue* Value;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
struct CJsonListNode {
	T Value;
	CJsonListNode<T>* Next;
};

template <class T>
class CJsonListIterator {
public:
	explicit CJsonListIterator( CJsonListNode<T>* first ) : current( first ) {}

	void operator++()
		{ current = current->Next; }
	T operator*() const
		{ return current->Value; }
	bool operator!=( CJsonListIterator other ) 
		{ return current != other.current; }

private:
	CJsonListNode<T>* current;
};

template <class T>
class CJsonList {
public:
	explicit CJsonList( CJsonListNode<T>* _first, int _size ) : first( _first ), size( _size ) {}

	int Size() const 
		{ return size; }

	CJsonListIterator<T> begin() const
		{ return CJsonListIterator<T>( first ); }
	CJsonListIterator<T> end() const
		{ return CJsonListIterator<T>( nullptr ); }

private:
	CJsonListNode<T>* first;
	int size;
};

//////////////////////////////////////////////////////////////////////////

// Exception thrown on value type mismatch or missing values.
class REAPI CJsonValueException : public CException {
public:
	CJsonValueException( const CJsonValueException& other )
		: errorText( copy( other.errorText ) ) {}
	CJsonValueException& operator=( const CJsonValueException& other );

	// Initialize a conversion error.
	CJsonValueException( TJsonValueType expected, TJsonValueType actual );
	// Initialize a missing object key error.
	explicit CJsonValueException( CStringPart missingKeyName );

	virtual CString GetMessageText() const override final;

private:
	CString errorText;

	static CStringView getValueTypeName( TJsonValueType type );
};

//////////////////////////////////////////////////////////////////////////

// Base value in the JSON hierarchy.
class CJsonValue {
public:
	TJsonValueType GetType() const
		{ return valueType; }

	bool IsNull() const
		{ return valueType == JVT_Null; }
	bool IsBool() const
		{ return valueType == JVT_Bool; }
	bool IsNumber() const
		{ return valueType == JVT_Number; }
	bool IsString() const
		{ return valueType == JVT_String; }
	bool IsArray() const
		{ return valueType == JVT_Array; }
	bool IsObject() const
		{ return valueType == JVT_Object; }

	// Quick access methods. Value type must correspond correctly to the requested value, otherwise an exception is thrown.
	double GetAsNumber() const;
	CStringPart GetAsString() const;
	bool GetAsBool() const;
	CJsonList<CJsonValue*> GetAsArray() const;
	CJsonValue& FindObjectValue( CStringPart key ) const;
	CJsonValue* TryFindObjectValue( CStringPart key ) const;
	CJsonList<CJsonKeyValue> GetObjectKeyValues() const;

protected:
	explicit CJsonValue( TJsonValueType type ) : valueType( type ) {}

private:
	TJsonValueType valueType;

	void checkValidConversion( TJsonValueType expected ) const;

	// Copying is prohibited.
	CJsonValue( CJsonValue& ) = delete;
	void operator=( CJsonValue& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

class CJsonNull : public CJsonValue {
public:
	CJsonNull() : CJsonValue( JVT_Null ) {}
};

//////////////////////////////////////////////////////////////////////////

class CJsonNumber : public CJsonValue {
public:
	explicit CJsonNumber( double number ) : CJsonValue( JVT_Number ), valueNumber( number ) {}

	double GetNumber() const
		{ return valueNumber; }

private:
	double valueNumber;
};

//////////////////////////////////////////////////////////////////////////

class CJsonBool : public CJsonValue {
public:
	explicit CJsonBool( bool boolVal ) : CJsonValue( JVT_Bool ), boolValue( boolVal ) {}

	bool GetBool() const
		{ return boolValue; }

private:
	bool boolValue;
};

//////////////////////////////////////////////////////////////////////////

class CJsonString : public CJsonValue {
public:
	explicit CJsonString( CStringPart str ) : CJsonValue( JVT_String ), stringValue( str ) {}
	
	CStringPart GetString() const
		{ return stringValue; }

private:
	CStringPart stringValue;	
};

//////////////////////////////////////////////////////////////////////////

class CJsonDynamicArray : public CJsonValue {
public:
	explicit CJsonDynamicArray( CJsonListNode<CJsonValue*>* head, CJsonListNode<CJsonValue*>* tail, int size ) :
		CJsonValue( JVT_Array ), listHead( head ), listTail( tail ), listSize( size ) {}

	int Size() const
		{ return listSize; }

	CJsonList<CJsonValue*> GetValues() const
		{ return CJsonList<CJsonValue*>( listHead, listSize ); }

	// Json document needs access to the underlying implementation in order to change values.
	friend class CJsonDocument;

private:
	CJsonListNode<CJsonValue*>* listHead;
	CJsonListNode<CJsonValue*>* listTail;
	int listSize;

	CJsonListNode<CJsonValue*>* getTail()
		{ return listTail; }
	void setTail( CJsonListNode<CJsonValue*>* newValue )
		{ listTail = newValue; listSize++; }
	void setFirstNode( CJsonListNode<CJsonValue*>* newValue )
		{ listHead = listTail = newValue; listSize = 1; }
};

//////////////////////////////////////////////////////////////////////////

class CJsonObject : public CJsonValue {
public:
	explicit CJsonObject( CJsonListNode<CJsonKeyValue>* head, CJsonListNode<CJsonKeyValue>* tail, int size ) : 
		CJsonValue( JVT_Object ), listHead( head ), listTail( tail ), listSize( size ) {}

	int Size() const
		{ return listSize; }

	CJsonList<CJsonKeyValue> GetKeyValueList() const
		{ return CJsonList<CJsonKeyValue>( listHead, listSize ); }

	// Return the value under the specified key or nullptr if no value exists.
	CJsonValue* TryFindValue( CStringPart keyName ) const;
	CJsonValue& FindValue( CStringPart keyName ) const;

	// Json document needs access to the underlying implementation in order to change values.
	friend class CJsonDocument;

private:
	CJsonListNode<CJsonKeyValue>* listHead;
	CJsonListNode<CJsonKeyValue>* listTail;
	int listSize;

	CJsonListNode<CJsonKeyValue>* getTail()
		{ return listTail; }
	void setTail( CJsonListNode<CJsonKeyValue>* newValue )
		{ listTail = newValue; listSize++; }
	void setFirstNode( CJsonListNode<CJsonKeyValue>* newValue )
		{ listHead = listTail = newValue; listSize = 1; }
};

//////////////////////////////////////////////////////////////////////////

inline CJsonValue* CJsonObject::TryFindValue( CStringPart keyName ) const
{
	for( auto keyVal : GetKeyValueList() ) {
		if( keyVal.Key == keyName ) {
			return keyVal.Value;
		}
	}
	return nullptr;
}

inline CJsonValue& CJsonObject::FindValue( CStringPart keyName ) const
{
	for( auto keyVal : GetKeyValueList() ) {
		if( keyVal.Key == keyName ) {
			return *keyVal.Value;
		}
	}
	throw CJsonValueException( keyName );
}

//////////////////////////////////////////////////////////////////////////

inline double CJsonValue::GetAsNumber() const
{
	checkValidConversion( JVT_Number );
	return static_cast<const CJsonNumber*>( this )->GetNumber();
}

inline CStringPart CJsonValue::GetAsString() const
{
	checkValidConversion( JVT_String );
	return static_cast<const CJsonString*>( this )->GetString();
}

inline bool CJsonValue::GetAsBool() const
{
	checkValidConversion( JVT_Bool );
	return static_cast<const CJsonBool*>( this )->GetBool();
}

inline CJsonList<CJsonValue*> CJsonValue::GetAsArray() const
{
	checkValidConversion( JVT_Array );
	return static_cast<const CJsonDynamicArray*>( this )->GetValues();
}

inline CJsonValue& CJsonValue::FindObjectValue( CStringPart key ) const
{
	checkValidConversion( JVT_Object );
	return static_cast<const CJsonObject*>( this )->FindValue( key );
}

inline CJsonValue* CJsonValue::TryFindObjectValue( CStringPart key ) const
{
	checkValidConversion( JVT_Object );
	return static_cast<const CJsonObject*>( this )->TryFindValue( key );
}

inline void CJsonValue::checkValidConversion( TJsonValueType expected ) const
{
	if( valueType != expected ) {
		throw CJsonValueException( expected, valueType );
	}
}

//////////////////////////////////////////////////////////////////////////

inline CJsonList<CJsonKeyValue> CJsonValue::GetObjectKeyValues() const
{
	assert( valueType == JVT_Object );
	return static_cast<const CJsonObject*>( this )->GetKeyValueList();
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.

