#include <Message.h>
#include <MessageSystem.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CMessage::CMessage( int _messageId ) :
	messageId( _messageId )
{
}

CMessage::CMessage( int sectionId, CUnicodePart messageName ) :
	messageId( CMessageSystemSwitcher::GetCurrentSystem()->FindMessageId( sectionId, messageName ) )
{
}

CMessage::CMessage( CStringPart sectionName, CUnicodePart messageName ) :
	messageId( CMessageSystemSwitcher::GetCurrentSystem()->FindMessageId( sectionName, messageName ) )
{
}

CUnicodeView CMessage::GetText() const
{
	assert( messageId >= 0 );
	const CMessageSystem* system = CMessageSystemSwitcher::GetCurrentSystem();
	return system->GetMessageById( messageId );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
