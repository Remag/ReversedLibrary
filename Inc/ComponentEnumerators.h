#pragma once
#include <InlineEntityStorage.h>
#include <Link.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for iteration through valid component ids.
class CComponentIdEnumerator {
public:
	typedef RelibInternal::CEntityIndexData TIndexData;
	typedef CArrayView<TIndexData> TIndexView;

	explicit CComponentIdEnumerator( TIndexView index, TIndexView overflowData ) : linkEnum( index.begin(), index.end(), overflowData.begin(), overflowData.end() ) {};
	explicit CComponentIdEnumerator( RelibInternal::CLinkEnumerator<const TIndexData*, const TIndexData*> _linkEnum ) : linkEnum( _linkEnum ) {}
	
	int operator*() const
		{ return numeric_cast<int>( ( *linkEnum ).ComponentId ); }
	void operator++()
		{ ++linkEnum; iterateToValidId(); }
	bool operator!=( CComponentIdEnumerator other )
		{ return other.linkEnum != linkEnum; }

	CComponentIdEnumerator begin() const
		{ CComponentIdEnumerator result( linkEnum.begin() ); result.iterateToValidId(); return result; }
	CComponentIdEnumerator end() const
		{ return CComponentIdEnumerator( linkEnum.end() ); }

private:
	RelibInternal::CLinkEnumerator<const TIndexData*, const TIndexData*> linkEnum;

	void iterateToValidId();
};

//////////////////////////////////////////////////////////////////////////

inline void CComponentIdEnumerator::iterateToValidId()
{
	const auto linkEnd = linkEnum.end();
	while( linkEnum != linkEnd ) {
		const auto currentEntry = *linkEnum;
		if( currentEntry.ComponentId != NotFound && currentEntry.ComponentId != USHRT_MAX ) {
			break;
		}
		++linkEnum;
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

