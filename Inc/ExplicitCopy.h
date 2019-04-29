#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

template <class Dest, class Src>
typename Types::EnableIf<!Types::HasCopyConstructor<Dest>::Result, Dest>::Result CreateCopy( const Src& ref );
template <class Dest, class Src>
typename Types::EnableIf<Types::HasCopyConstructor<Dest>::Result, Dest>::Result CreateCopy( const Src& ref );

} // namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// A copy tag.
// Objects that wish to be explicitly copied must define a constructor that takes a const reference to an object and a const reference to this tag.
class CExplicitCopyTag {
public:
	template <class Dest, class Src>
	friend typename Types::EnableIf<!Types::HasCopyConstructor<Dest>::Result, Dest>::Result RelibInternal::CreateCopy( const Src& ref );
	template <class Dest, class Src>
	friend typename Types::EnableIf<Types::HasCopyConstructor<Dest>::Result, Dest>::Result RelibInternal::CreateCopy( const Src& ref );

private:
	CExplicitCopyTag() = default;

	// Copying is prohibited.
	CExplicitCopyTag( CExplicitCopyTag& ) = delete;
	void operator=( CExplicitCopyTag& ) = delete;
};

// Utilities for better explicit copying.
//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class Dest, class Src>
typename Types::EnableIf<!Types::HasCopyConstructor<Dest>::Result, Dest>::Result CreateCopy( const Src& ref )
{
	return Dest( ref, CExplicitCopyTag() );
}

template <class Dest, class Src>
typename Types::EnableIf<Types::HasCopyConstructor<Dest>::Result, Dest>::Result CreateCopy( const Src& ref )
{
	return Dest( ref );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// An explicit copy function.
// The object itself is constructed using an object const reference with the copy tag.
template <class Dest, class Src>
Dest CopyTo( const Src& src )
{
	return RelibInternal::CreateCopy<Dest>( src );
}

// A simple copy of an object, where the result has the same type.
template <class T>
auto copy( const T& obj )
{
	return CopyTo<T>( obj );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

