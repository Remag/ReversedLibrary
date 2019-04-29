#include <StackArray.h>
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A single item in a dictionary.
template <class EnumType, class ValueType>
struct CEnumItem {
	EnumType Type;
	ValueType Name;

	template <class Value>
	CEnumItem( EnumType type, Value name ) : Type( type ), Name( move( name ) ) {}
};

//////////////////////////////////////////////////////////////////////////

// Class for converting enum values to unicode strings.
// One enum value corresponds to exactly one string.
template <class EnumType, int enumSize, class ValueType>
class CEnumDictionary {
public:
	CEnumDictionary() = default;
	explicit CEnumDictionary( std::initializer_list<CEnumItem<EnumType, ValueType>> stringList );

	void Set( EnumType type, ValueType val )
		{ items[type] = move( val ); }

	// Get the string corresponding to the given value. 
	// Default string is empty.
	const ValueType& GetValue( EnumType type ) const
		{ assert( type < enumSize ); return items[type]; }
	const ValueType& operator[]( EnumType type ) const
		{ return GetValue( type ); }

	template <class Value>
	EnumType FindEnum( const Value& val ) const;
	template <class Value>
	EnumType FindEnum( const Value& val, EnumType defaultValue ) const;
	
	template <class Value, class Comparator>
	EnumType FindEnum( const Value& val, Comparator comp ) const;
	template <class Value, class Comparator>
	EnumType FindEnum( const Value& val, Comparator comp, EnumType defaultValue ) const;

private:
	CStackArray<ValueType, enumSize> items;
};

//////////////////////////////////////////////////////////////////////////

template <class EnumType, int enumSize, class ValueType>
CEnumDictionary<EnumType, enumSize, ValueType>::CEnumDictionary( std::initializer_list<CEnumItem<EnumType, ValueType>> stringList )
{
	for( const auto& elem : stringList ) {
		items[elem.Type] = elem.Name;
	}
}

template <class EnumType, int enumSize, class ValueType>
template <class Value>
EnumType CEnumDictionary<EnumType, enumSize, ValueType>::FindEnum( const Value& val ) const
{
	for( int i = 0; i < enumSize; i++ ) {
		if( items[i] == val ) {
			return EnumType( i );
		}
	}
	assert( false );
	return EnumType( 0 );

}

template <class EnumType, int enumSize, class ValueType>
template <class Value>
EnumType CEnumDictionary<EnumType, enumSize, ValueType>::FindEnum( const Value& val, EnumType defaultValue ) const
{
	for( int i = 0; i < enumSize; i++ ) {
		if( items[i] == val ) {
			return EnumType( i );
		}
	}
	return defaultValue;
}

template <class EnumType, int enumSize, class ValueType>
template <class Value, class Comparator>
EnumType CEnumDictionary<EnumType, enumSize, ValueType>::FindEnum( const Value& val, Comparator comp ) const
{
	for( int i = 0; i < enumSize; i++ ) {
		if( comp( items[i], val ) ) {
			return EnumType( i );
		}
	}
	assert( false );
	return EnumType( 0 );
}

template <class EnumType, int enumSize, class ValueType>
template <class Value, class Comparator>
EnumType CEnumDictionary<EnumType, enumSize, ValueType>::FindEnum( const Value& val, Comparator comp, EnumType defaultValue ) const
{
	for( int i = 0; i < enumSize; i++ ) {
		if( comp( items[i], val ) ) {
			return EnumType( i );
		}
	}
	return defaultValue;
}
	
//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.