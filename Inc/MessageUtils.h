#pragma once
#include <Redefs.h>
#include <HashTable.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class representing a subsystem that can send messages.
class CMessageSource {
public:
	template <class T>
	CMessageSource( const T& subsystem ) : info( &typeid( subsystem ) ) { subsystem; }
	// Pointers to subsystems are treated as references.
	template <class T>
	CMessageSource( const T* subsystem ) : info( &typeid( *subsystem ) ) { subsystem; }
	template <class T>
	CMessageSource( T* subsystem ) : info( &typeid( *subsystem ) ) { subsystem; }

	const type_info* GetTypeInfo() const
		{ return info; }

private:
	const type_info* info;
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Class for filtering messages based on their source.
// Every thread has a personal filter.
class REAPI CMessageFilter {
public:
	// Clear the filter value for the current thread.
	~CMessageFilter();

	// Clear all current filters.
	void Empty();

	// Check if a given message can pass through the filter.
	bool IsMessageAllowed( CMessageSource src );

	// Add a value to the whitelist and turn whitelist mode on.
	void AddToWhitelist( CMessageSource src, bool isSet );
	// Add a value to the blacklist and turn blacklist mode on.
	void AddToBlacklist( CMessageSource src, bool isSet );

	// Set whitelist mode.
	void SetWhiteList( bool isSet )
		{ isFilterWhitelist = isSet; }

private:
	CHashTable<const type_info*> filteredValues;
	// Current filtering mode. True means only values preset in the hash table are allowed.
	// False means all values in the table are not allowed.
	bool isFilterWhitelist = false;

	void setFilter( CMessageSource src, bool isSet );
};

//////////////////////////////////////////////////////////////////////////

// Mechanism that makes sure that message filter is initialized for every thread during static initialization.
class REAPI CMessageFilterInitializer {
public:
	CMessageFilterInitializer();
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

namespace Log {

// Check if a given message source is currently allowed by the filter.
bool REAPI IsMessageSourceShown( CMessageSource src );
// Add or remove a given source from a blacklist. If whitelist mode was on, it's disabled and all previous filters are cleared.
void REAPI BlacklistSource( CMessageSource src, bool isBlacklisted );
// Add or remove given source from a whitelist. If blacklist mode was on, it's disabled and all previous filters are cleared.
void REAPI WhitelistSource( CMessageSource src, bool isWhitelisted );

// Show all messages.
void REAPI WhitelistAll();
// Hide all messages. Errors are still shown.
void REAPI BlacklistAll();

}	// namespace Log.

}	// namespace Relib.

