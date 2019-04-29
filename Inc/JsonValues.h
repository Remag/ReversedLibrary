#pragma once
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <RawStringBuffer.h>
#include <StrConversions.h>
#include <ArrayBuffer.h>

namespace Relib {

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

template <class T>
struct CJsonListNode {
	T Value;
	CJsonListNode<T>* Next;
};

template <class T>
class CJsonListEnumerator {
public:
	explicit CJsonListEnumerator( CJsonListNode<T>* first ) : current( first ) {}

	CJsonListEnumerator begin() const
		{ return CJsonListEnumerator( current ); }
	CJsonListEnumerator end() const
		{ return CJsonListEnumerator( nullptr ); }

	void operator++()
		{ current = current->Next; }
	T operator*() const
		{ return current->Value; }
	bool operator!=( CJsonListEnumerator other ) 
		{ return current != other.current; }

private:
	CJsonListNode<T>* current;
};

//////////////////////////////////////////////////////////////////////////

// Base value in the JSON hierarchy.
class CJsonValue {
public:
	TJsonValueType GetType() const
		{ return valueType; }

	// Quick access methods. Value type must correspond correctly to the requested value.
	double GetAsNumber() const;
	CStringPart GetAsString() const;
	bool GetAsBool() const;
	CJsonListEnumerator<CJsonValue*> GetAsArray() const;
	CJsonValue* FindObjectValue( CStringPart key ) const;

protected:
	explicit CJsonValue( TJsonValueType type ) : valueType( type ) {}

private:
	TJsonValueType valueType;
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

	CJsonListEnumerator<CJsonValue*> GetValues() const
		{ return CJsonListEnumerator<CJsonValue*>( listHead ); }

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

struct CJsonKeyValue {
	CStringPart Key;
	CJsonValue* Value;
};

class CJsonObject : public CJsonValue {
public:
	explicit CJsonObject( CJsonListNode<CJsonKeyValue>* head, CJsonListNode<CJsonKeyValue>* tail, int size ) : 
		CJsonValue( JVT_Object ), listHead( head ), listTail( tail ), listSize( size ) {}

	int Size() const
		{ return listSize; }

	CJsonListEnumerator<CJsonKeyValue> GetKeyValueList() const
		{ return CJsonListEnumerator<CJsonKeyValue>( listHead ); }

	// Return the value under the specified key or nullptr if no value exists.
	CJsonValue* FindValue( CStringPart keyName ) const;

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

inline CJsonValue* CJsonObject::FindValue( CStringPart keyName ) const
{
	for( auto keyVal : GetKeyValueList() ) {
		if( keyVal.Key == keyName ) {
			return keyVal.Value;
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

inline double CJsonValue::GetAsNumber() const
{
	assert( valueType == JVT_Number );
	return static_cast<const CJsonNumber*>( this )->GetNumber();
}

inline CStringPart CJsonValue::GetAsString() const
{
	assert( valueType == JVT_String );
	return static_cast<const CJsonString*>( this )->GetString();
}

inline bool CJsonValue::GetAsBool() const
{
	assert( valueType == JVT_Bool );
	return static_cast<const CJsonBool*>( this )->GetBool();
}

inline CJsonListEnumerator<CJsonValue*> CJsonValue::GetAsArray() const
{
	assert( valueType == JVT_Array );
	return static_cast<const CJsonDynamicArray*>( this )->GetValues();
}

inline CJsonValue* CJsonValue::FindObjectValue( CStringPart key ) const
{
	assert( valueType == JVT_Object );
	return static_cast<const CJsonObject*>( this )->FindValue( key );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.

