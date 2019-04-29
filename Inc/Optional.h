#pragma once
#include <Redefs.h>
#include <TemplateUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class that may or may not contain an object of a given type.
template <class InnerType>
class COptional {
public:
	// Default constructor. Creates an invalid optional.
	COptional();
	// Construct an optional value with the given arguments.
	template <class FirstArg, class... Args>
	explicit COptional( FirstArg&& arg, Args&&... contructorArgs );

	// Copying and movement.
	COptional( const COptional& other );
	COptional( COptional& other );
	COptional( COptional&& other );
	COptional& operator=( COptional other );
	// Call the destructor of an inner type if it's valid.
	~COptional();

	// Does this optional contain a valid value.
	bool IsValid() const
		{ return isValid; }
	// Return the value. Null pointer is returned if value is not valid.
	InnerType* GetValue();
	const InnerType* GetValue() const
		{ return const_cast<COptional<InnerType>*>( this )->GetValue(); }
	// Copy/move a given value.
	void SetValue( InnerType newValue );
	COptional& operator=( InnerType newValue );
	// Delete an existing value or do nothing if the value is invalid.
	void DeleteValue();
	// Create value, using a constructor with the given arguments.
	// An old value is deleted.
	template <class ...Args>
	InnerType& CreateValue( Args&&... args );

	// Value access through operators. Value is asserted to be valid.
	InnerType& operator*();
	const InnerType& operator*() const
		{ return const_cast<COptional<InnerType>*>( this )->operator*(); }
	InnerType* operator->();
	const InnerType* operator->() const
		{ return const_cast<COptional<InnerType>*>( this )->operator->(); }

private:
	// Memory containing an optional object.
	alignas( InnerType ) BYTE innerTypeData[sizeof( InnerType )];
	// Does the data contain an actual value.
	bool isValid;

	InnerType& getTypeFromData();
	const InnerType& getTypeFromData() const;

	template <class... Args>
	void createData( Args&&... args );
};

//////////////////////////////////////////////////////////////////////////

template <class InnerType>
COptional<InnerType>::COptional() :
	isValid( false )
{
}

template <class InnerType>
template <class FirstArg, class... Args>
COptional<InnerType>::COptional( FirstArg&& arg, Args&&... contructorArgs ) :
	isValid( true )
{
	createData( forward<FirstArg>( arg ), forward<Args>( contructorArgs )... );
}

template <class InnerType>
COptional<InnerType>::COptional( const COptional<InnerType>& other ) :
	isValid( other.isValid )
{
	if( other.isValid ) {
		createData( other.getTypeFromData() );
	}
}

template <class InnerType>
COptional<InnerType>::COptional( COptional<InnerType>& other ) :
	isValid( other.isValid )
{
	if( other.isValid ) {
		createData( other.getTypeFromData() );
	}
}

template <class InnerType>
COptional<InnerType>::COptional( COptional<InnerType>&& other ) :
	isValid( other.isValid )
{
	if( other.isValid ) {
		createData( move( other.getTypeFromData() ) );
		other.isValid = false;
	}
}

template <class InnerType>
COptional<InnerType>& COptional<InnerType>::operator=( COptional<InnerType> other )
{
	swap( *this, other );
	return *this;
}

template <class InnerType>
COptional<InnerType>::~COptional()
{
	if( isValid ) {
		getTypeFromData().~InnerType();
	}
}

template <class InnerType>
InnerType* COptional<InnerType>::GetValue()
{
	return IsValid() ? &getTypeFromData() : 0;
}

template <class InnerType>
InnerType& COptional<InnerType>::operator*()
{
	assert( IsValid() );
	return getTypeFromData();
}
	
template <class InnerType>
InnerType* COptional<InnerType>::operator->()
{
	assert( IsValid() );
	return &getTypeFromData();
}

template <class InnerType>
void COptional<InnerType>::SetValue( InnerType newValue )
{
	if( isValid ) {
		getTypeFromData() = move( newValue );
	} else {
		createData( move( newValue ) );
		isValid = true;
	}
}

template <class InnerType>
COptional<InnerType>& COptional<InnerType>::operator=( InnerType newValue )
{
	SetValue( move( newValue ) );
	return *this;
}

template <class InnerType>
void COptional<InnerType>::DeleteValue()
{
	if( !isValid ) {
		return;
	}
	getTypeFromData().~InnerType();
	isValid = false;
}

template <class InnerType>
template <class ...Args>
InnerType& COptional<InnerType>::CreateValue( Args&&... args )
{
	if( isValid ) {
		getTypeFromData().~InnerType();
	}
	InnerType& result = *new( innerTypeData ) InnerType( forward<Args>( args )... );
	isValid = true;
	return result;
}

template <class InnerType>
template <class... Args>
void COptional<InnerType>::createData( Args&&... args )
{
	new( innerTypeData ) InnerType( forward<Args>( args )... );
}

// Access the previously constructed value in the memory buffer.
template <class InnerType>
InnerType& COptional<InnerType>::getTypeFromData()
{
	assert( isValid );
	return *reinterpret_cast<InnerType*>( innerTypeData );
}

template <class InnerType>
const InnerType& Relib::COptional<InnerType>::getTypeFromData() const
{
	assert( isValid );
	return *reinterpret_cast<const InnerType*>( innerTypeData );
}

template <class InnerType>
void swap( COptional<InnerType>& left, COptional<InnerType>& right )
{
	if( left.IsValid() && right.IsValid() ) {
		// Swap the valid values.
		swap( *left, *right );
	} else if( left.IsValid() ) {
		// Move the left value to the right.
		right = move( *left );
		left.DeleteValue();
	} else if( right.IsValid() ) {
		// Move the right value to the left.
		left = move( *right );
		right.DeleteValue();
	} else {
		// Both values are invalid, no need to do anything.
	}
}

template <class T>
COptional<T> CreateOptional( T val )
{
	return COptional<T>( move( val ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

