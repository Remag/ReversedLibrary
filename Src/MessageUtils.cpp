#include <MessageUtils.h>
#include <Reassert.h>
#include <RawStringBuffer.h>
#include <StrConversions.h>
#include <BaseString.h>
#include <BaseStringView.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

thread_local bool isFilterValid = true;
static CMessageFilter& getCurrentMessageFilter()
{
	assert( isFilterValid );
	thread_local CMessageFilter filter;
	return filter;
}

CMessageFilter::~CMessageFilter()
{
	assert( isFilterValid );
	isFilterValid = false;
}

void CMessageFilter::Empty()
{
	filteredValues.Empty();
}

bool CMessageFilter::IsMessageAllowed( CStringPart src )
{
	const bool filterHasMessage = filteredValues.HasValue( src );
	return filterHasMessage == isFilterWhitelist;
}

void CMessageFilter::AddToWhitelist( CStringPart src, bool isSet )
{
	if( !isFilterWhitelist ) {
		filteredValues.Empty();
		isFilterWhitelist = true;
	}
	setFilter( src, isSet );
}

void CMessageFilter::AddToBlacklist( CStringPart src, bool isSet )
{
	if( isFilterWhitelist ) {
		filteredValues.Empty();
		isFilterWhitelist = false;
	}

	setFilter( src, isSet );
}

void CMessageFilter::setFilter( CStringPart src, bool isSet )
{
	if( isSet ) {
		filteredValues.Set( Str( src ) );
	} else {
		filteredValues.Delete( src );
	}
}

//////////////////////////////////////////////////////////////////////////

thread_local CMessageFilterInitializer inititalizer;
CMessageFilterInitializer::CMessageFilterInitializer()
{
	getCurrentMessageFilter();
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

namespace Log {

bool IsMessageSourceShown( CStringPart src )
{
	if( !RelibInternal::isFilterValid ) {
		// Thread filter has already been destroyed, all messages are allowed.
		return true;
	}

	return RelibInternal::getCurrentMessageFilter().IsMessageAllowed( src );
}

void BlacklistSource( CStringPart src, bool isBlacklisted )
{
	RelibInternal::getCurrentMessageFilter().AddToBlacklist( src, isBlacklisted );
}

void WhitelistSource( CStringPart src, bool isWhitelisted )
{
	RelibInternal::getCurrentMessageFilter().AddToWhitelist( src, isWhitelisted );
}

void WhitelistAll()
{
	auto& filter = RelibInternal::getCurrentMessageFilter();
	filter.Empty();
	filter.SetWhiteList( false );
}

void BlacklistAll()
{
	auto& filter = RelibInternal::getCurrentMessageFilter();
	filter.Empty();
	filter.SetWhiteList( true );
}

}	// namespace Log.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
