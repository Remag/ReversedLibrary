#pragma once

#include <BaseString.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Locale dependent unicode string.
class REAPI CMessage {
public:
	// Create an uninitialized message.
	CMessage() = default;
	// Create a message by its unique identifier.
	// List of all possible identifiers is available in the auto generated file: messages.h
	explicit CMessage( int messageId );
	// Create a message using its name and an identifier of its section.
	// This constructor must be run after the message system has been initialized.
	CMessage( int sectionId, CUnicodePart messageName );
	// Create a message using its name and its section name.
	// This constructor must be run after the message system has been initialized.
	CMessage( CStringPart sectionName, CUnicodePart messageName );
	
	// Message content. Gets fetched from the currently active message system.
	CUnicodeView GetText() const;

private:
	// Unique message identifier.
	int messageId = NotFound;
};


//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

