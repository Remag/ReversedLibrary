#include <MessageSystem.h>
#include <FileOwners.h>
#include <Archive.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

static int getMessageHashKey( CUnicodePart name, int sectionId )
{
	return CombineHashKey( GetUnicodeHash( name ), sectionId );
}

int CMessageSystem::CNamedMessageView::HashKey() const
{
	return getMessageHashKey( MessageName, SectionId );
}

int CMessageSystem::CNamedMessageOwner::HashKey() const
{
	return getMessageHashKey( MessageName, SectionId );
}

//////////////////////////////////////////////////////////////////////////

void CMessageSystem::LoadMessages( CUnicodeView fileName )
{
	CFileReader file( fileName, FCM_OpenExisting );
	CArchiveReader messageArchive( file );
	parseSectionList( messageArchive );
	parseMessages( messageArchive );
}

void CMessageSystem::parseSectionList( CArchiveReader& messageArchive )
{
	messageArchive >> sectionNames;
}

void CMessageSystem::parseMessages( CArchiveReader& messageArchive )
{
	int messageCount;
	messageArchive >> messageCount;
	messages.ReserveBuffer( messageCount );
	for( int i = 0; i < messageCount; i++ ) {
		CUnicodeString messageStr;
		messageArchive >> messageStr;
		messages.Add( move( messageStr ) );
	}

	while( !messageArchive.IsEndOfArchive() ) {
		CUnicodeString newMessageName;
		messageArchive >> newMessageName;

		int newSectionId;
		messageArchive >> newSectionId;

		int messageId;
		messageArchive >> messageId;
		CNamedMessageOwner newMessage( move( newMessageName ), newSectionId );
		namedMessages.Set( move( newMessage ), messageId );
	}
}

int CMessageSystem::FindMessageId( int sectionId, CUnicodePart messageName ) const
{
	const CNamedMessageView messageView{ messageName, sectionId };
	return namedMessages[messageView];
}

int CMessageSystem::FindMessageId( CStringPart sectionName, CUnicodePart messageName ) const
{
	const auto sectionId = sectionNames[sectionName];
	return FindMessageId( sectionId, messageName );
}

//////////////////////////////////////////////////////////////////////////

const CMessageSystem* CMessageSystemSwitcher::currentSystem;

CMessageSystemSwitcher::CMessageSystemSwitcher( const CMessageSystem& newSystem ) :
	prevSystem( currentSystem )
{
	currentSystem = &newSystem;
}

CMessageSystemSwitcher::~CMessageSystemSwitcher()
{
	currentSystem = prevSystem;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
