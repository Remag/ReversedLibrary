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

bool CMessageFilter::IsMessageAllowed( CMessageSource src )
{
	const bool filterHasMessage = filteredValues.HasValue( src.GetTypeInfo() );
	return filterHasMessage == isFilterWhitelist;
}

void CMessageFilter::AddToWhitelist( CMessageSource src, bool isSet )
{
	if( !isFilterWhitelist ) {
		filteredValues.Empty();
		isFilterWhitelist = true;
	}

	setFilter( src, isSet );
}

void CMessageFilter::AddToBlacklist( CMessageSource src, bool isSet )
{
	if( isFilterWhitelist ) {
		filteredValues.Empty();
		isFilterWhitelist = false;
	}

	setFilter( src, isSet );
}

void CMessageFilter::setFilter( CMessageSource src, bool isSet )
{
	if( isSet ) {
		filteredValues.Set( src.GetTypeInfo() );
	} else {
		filteredValues.Delete( src.GetTypeInfo() );
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

bool IsMessageSourceShown( CMessageSource src )
{
	if( !RelibInternal::isFilterValid ) {
		// Thread filter has already been destroyed, all messages are allowed.
		return true;
	}

	return RelibInternal::getCurrentMessageFilter().IsMessageAllowed( src );
}

void BlacklistSource( CMessageSource src, bool isBlacklisted )
{
	RelibInternal::getCurrentMessageFilter().AddToBlacklist( src, isBlacklisted );
}

void WhitelistSource( CMessageSource src, bool isWhitelisted )
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
