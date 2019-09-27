#pragma once

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class StrType, class SearchType>
class CStringSearchEnumerator {
public:
	CStringSearchEnumerator() = default;
	CStringSearchEnumerator( StrType _str, SearchType _delimiter ) : str( _str ), delimiter( _delimiter ), pos( searchStrPos( 0 ) ) {}

	void operator++() 
		{ pos = searchStrPos( pos + 1 ); }
	auto operator*()
		{ return pos; }

	bool operator!=( CStringSearchEnumerator<StrType, SearchType> other )
		{ return pos != other.pos; }

	CStringSearchEnumerator<StrType, SearchType> begin() const
		{ return *this; }
	CStringSearchEnumerator<StrType, SearchType> end() const
		{ return CStringSearchEnumerator<StrType, SearchType>(); }

private:
	StrType str;
	SearchType delimiter;
	int pos = NotFound;

	int searchStrPos( int pos ) const
		{ return str.Find( delimiter, pos ); }
};

//////////////////////////////////////////////////////////////////////////

template <class StrType, class SearchType>
class CNoCaseStringSearchEnumerator {
public:
	CNoCaseStringSearchEnumerator() = default;
	CNoCaseStringSearchEnumerator( StrType _str, SearchType _delimiter ) : str( _str ), delimiter( _delimiter ), pos( searchStrPos( 0 ) ) {}

	void operator++() 
		{ pos = searchStrPos( pos + 1 ); }
	auto operator*()
		{ return pos; }

	bool operator!=( CNoCaseStringSearchEnumerator<StrType, SearchType> other )
		{ return pos != other.pos; }

	CNoCaseStringSearchEnumerator<StrType, SearchType> begin() const
		{ return *this; }
	CNoCaseStringSearchEnumerator<StrType, SearchType> end() const
		{ return CNoCaseStringSearchEnumerator<StrType, SearchType>(); }

private:
	StrType str;
	SearchType delimiter;
	int pos = NotFound;

	int searchStrPos( int searchPos ) const
		{ return str.FindNoCase( delimiter, searchPos ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

