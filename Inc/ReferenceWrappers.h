#pragma once
#include <Reutils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A simple copyable reference holder.
template <class T>
class CRef {
public:
	CRef( T& value ) : refValue( AddressOf( value ) ) {}

	T& GetRef() const
		{ return *refValue; }

	operator T&()
		{ return *refValue; }
	operator const T&() const
		{ return *refValue; }

private:
	T* refValue;
};

//////////////////////////////////////////////////////////////////////////

// A simple copyable constant reference holder.
template <class T>
class CConstRef {
public:
	CConstRef( const T& value ) : refValue( AddressOf( value ) ) {}

	const T& GetRef() const
		{ return *refValue; }

	operator const T&() const
		{ return *refValue; }

private:
	const T* refValue;
};

//////////////////////////////////////////////////////////////////////////

// Wrapper creation functions.
template <class T>
CRef<T> Ref( T& ref )
{
	return CRef<T>( ref );
}

template <class T>
CConstRef<T> ConstRef( const T& ref )
{
	return CConstRef<T>( ref );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

