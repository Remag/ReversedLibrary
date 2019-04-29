#pragma once
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class T>
typename Types::EnableIf<Types::IsFundamental<T>::Result, int>::Result getSplitDelimiterSize( T )
{
	return 1;
}

template <class T>
typename Types::EnableIf<!Types::IsFundamental<T>::Result, int>::Result getSplitDelimiterSize( T delimiter )
{
	return delimiter.Length();
}

//////////////////////////////////////////////////////////////////////////

// String enumeration mechanism. Provides range-based for loop support.
template <class StrType, class DelimiterType>
class CSplitEnumerator {
public:
	CSplitEnumerator() = default;
	CSplitEnumerator( StrType _str, DelimiterType _delimiter ) : str( _str ), delimiter( _delimiter ) {}

	void operator++() 
		{ pos = nextPos + getSplitDelimiterSize( delimiter ); }
	auto operator*()
		{ nextPos = findNextPos( pos ); return str.Mid( pos, nextPos - pos ); }

	bool operator!=( CSplitEnumerator<StrType, DelimiterType> )
		{ return pos < str.Length(); }

	CSplitEnumerator<StrType, DelimiterType> begin() const
		{ return *this; }
	CSplitEnumerator<StrType, DelimiterType> end() const
		{ return CSplitEnumerator<StrType, DelimiterType>(); }

private:
	StrType str;
	DelimiterType delimiter;
	int pos = 0;
	int nextPos = 0;

	int findNextPos( int pos ) const;
};

//////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable: 4180 )	// qualifier applied to function type has no meaning; ignored

// Condition based split enumerator.
template <class StrType, class SkipOperation>
class CActionSplitEnumerator {
public:
	CActionSplitEnumerator() = default;
	CActionSplitEnumerator( StrType _str, const SkipOperation& _skipAction ) : str( _str ), skipAction( _skipAction ) {}

	void operator++()
		{ pos = nextPos + delimiterLength; }
	auto operator*()
		{ findNextPos( pos ); return str.Mid( pos, nextPos - pos ); }

	bool operator!=( CActionSplitEnumerator<StrType, SkipOperation> )
		{ return pos < str.Length(); }

	CActionSplitEnumerator<StrType, SkipOperation> begin() const
		{ return *this; }
	CActionSplitEnumerator<StrType, SkipOperation> end() const
		{ return CActionSplitEnumerator<StrType, SkipOperation>( str, skipAction ); }

private:
	StrType str;
	const SkipOperation& skipAction;
	int pos = 0;
	int nextPos = 0;
	int delimiterLength = 0;

	void findNextPos( int pos );
};

#pragma warning( pop )

//////////////////////////////////////////////////////////////////////////

template <class StrType, class DelimiterType>
int CSplitEnumerator<StrType, DelimiterType>::findNextPos( int prevPos ) const
{
	const int next = str.Find( delimiter, prevPos );
	return next == NotFound ? str.Length() : next;
}

template <class StrType, class SkipOperation>
void CActionSplitEnumerator<StrType, SkipOperation>::findNextPos( int prevPos )
{
	const auto length = str.Length();
	for( int i = prevPos; i < length; i++ ) {
		const auto substr = str.Mid( i );
		const int currDelim = skipAction( substr );
		if( currDelim > 0 ) {
			nextPos = i;
			delimiterLength = currDelim;
			return;
		}
	}

	nextPos = length;
	delimiterLength = 0;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

