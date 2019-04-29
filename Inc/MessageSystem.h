#pragma once

#include <Array.h>
#include <Map.h>
#include <BaseString.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class for parsing and handling messages contained in a binary file that is created by the message compiler.
class REAPI CMessageSystem {
public:
	// Default constructions. Messages are left uninitialized.
	CMessageSystem() = default;

	// Load the messages from the given binary file.
	void LoadMessages( CUnicodeView fileName );

	int FindMessageId( int sectionId, CUnicodePart messageName ) const;
	int FindMessageId( CStringPart sectionName, CUnicodePart messageName ) const;
	CUnicodeView GetMessageById( int messageId ) const
		{ return messages[messageId]; }

private:
	// A named message structure. This messages are defined by their name string and their section identifier.
	struct CNamedMessageView {
		CUnicodePart MessageName;
		int SectionId;

		int HashKey() const;
	};

	struct CNamedMessageOwner {
		CUnicodeString MessageName;
		int SectionId;

		CNamedMessageOwner() = default;
		CNamedMessageOwner( CUnicodeString name, int _sectionId ) : MessageName( move( name ) ), SectionId( _sectionId ) {}

		int HashKey() const;
	};

	struct CMessageHash {
		static int HashKey( CNamedMessageView view )
			{ return view.HashKey(); }
		static int HashKey( const CNamedMessageOwner& owner )
			{ return owner.HashKey(); }

		template <class Left, class Right>
		static bool IsEqual( const Left& left, const Right& right )
			{ return left.MessageName == right.MessageName && left.SectionId == right.SectionId; }
	};

	// All the messages in the system.
	CArray<CUnicodeString> messages;
	// Dictionary of all the named messages. Message unique id is the value.
	CMap<CNamedMessageOwner, int, CMessageHash> namedMessages;
	// Relation between section names and identifiers.
	CMap<CString, int> sectionNames;

	void parseSectionList( CArchiveReader& messageArchive );
	void parseMessages( CArchiveReader& messageArchive );

	// Copying is prohibited.
	CMessageSystem( CMessageSystem& ) = delete;
	void operator=( CMessageSystem& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Switcher for the active message system. All user messages fetch their values from an active system.
class REAPI CMessageSystemSwitcher {
public:
	explicit CMessageSystemSwitcher( const CMessageSystem& newSystem );
	~CMessageSystemSwitcher();

	static const CMessageSystem* GetCurrentSystem()
		{ assert( currentSystem != 0 ); return currentSystem; }

private:
	const CMessageSystem* prevSystem;

	static const CMessageSystem* currentSystem;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

