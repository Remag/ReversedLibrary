#pragma once
#include <HashUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A pair of arbitrary items. Provides public access to both of them.
template <class FirstType, class SecondType /*= FirstType*/>
struct CPair {
	FirstType First;
	SecondType Second;

	CPair() = default;
	// Construct FirstType and SecondType with given arguments of any type.
	template<class FirstArg, class SecondArg>
	CPair( FirstArg&& _first, SecondArg&& _second ) : First( forward<FirstArg>( _first ) ), Second( forward<SecondArg>( _second ) ) {}
	// Explicit copying.
	CPair( const CPair<FirstType, SecondType>& other, CExplicitCopyTag&& ) : First( copy( other.First ) ), Second( copy( other.Second ) ) {}

	int HashKey() const;
	template <class OtherFirst, class OtherSecond>
	bool operator==( const CPair<OtherFirst, OtherSecond>& other ) const
		{ return First == other.First && Second == other.Second; }
};

//////////////////////////////////////////////////////////////////////////

template <class FirstType, class SecondType>
int CPair<FirstType, SecondType>::HashKey() const
{
	int firstKey = CDefaultHash<FirstType>::HashKey( First );
	return CombineHashKey( CDefaultHash<SecondType>::HashKey( Second ), firstKey );
}

//////////////////////////////////////////////////////////////////////////

template <class FirstType, class SecondType>
auto CreatePair( FirstType&& first, SecondType&& second )
{
	return CPair<typename Types::PureType<FirstType>::Result, typename Types::PureType<SecondType>::Result>{ forward<FirstType>( first ), forward<SecondType>( second ) };
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

