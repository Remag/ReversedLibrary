#include <MessageLog.h>
#include <MessageUtils.h>
#include <Errors.h>
#include <MessageLogImpls.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace Log {

extern CWindowMessageLog WindowLog;
extern CStdOutputLog StdLog;
static IMessageLog* defaultLog = nullptr;
static IMessageLog& getDefaultMessageLog()
{
	if( defaultLog != nullptr ) {
		return *defaultLog;
	}

	const int stdType = ::GetFileType( ::GetStdHandle( STD_OUTPUT_HANDLE ) );
	const int fileType = stdType & ~FILE_TYPE_REMOTE;
	if( fileType == FILE_TYPE_CHAR || fileType == FILE_TYPE_DISK || fileType == FILE_TYPE_PIPE ) {
		defaultLog = &StdLog;
	} else {
		defaultLog = &WindowLog;
	}
	return *defaultLog;
}

thread_local IMessageLog* currentMessageLog = nullptr;
static IMessageLog& getCurrentMessageLog()
{
	return currentMessageLog == nullptr ? getDefaultMessageLog() : *currentMessageLog;
}

extern thread_local CStringView CurrentMessageSource;
static CStringView getCurrentMessageSource()
{
	return CurrentMessageSource;
}

//////////////////////////////////////////////////////////////////////////

void Exception( const CException& e )
{
	if( IsMessageSourceShown( getCurrentMessageSource() ) ) {
		getCurrentMessageLog().AddMessage( e.GetMessageText(), LMT_Exception );
	}
}

void CriticalException( const CException& e )
{
	if( IsMessageSourceShown( getCurrentMessageSource() ) ) {
		getCurrentMessageLog().AddMessage( e.GetMessageText(), LMT_CriticalException );
	}
}

}	// namespace Log.

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

void sendLogMessage( TLogMessageType msgType, CString text )
{
	if( Log::IsMessageSourceShown( Log::getCurrentMessageSource() ) ) {
		Log::getCurrentMessageLog().AddMessage( move( text ), LMT_Periodic );
	}
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

CMessageLogSwitcher::CMessageLogSwitcher( IMessageLog& newValue ) :
	prevLog( Log::currentMessageLog )
{
	Log::currentMessageLog = &newValue;
}

CMessageLogSwitcher::~CMessageLogSwitcher()
{
	Log::currentMessageLog = prevLog;
}

//////////////////////////////////////////////////////////////////////////

CMessageSourceSwitcher::CMessageSourceSwitcher( CStringPart srcName ) :
	msgSource( srcName ),
	prevSource( Log::CurrentMessageSource )
{
	Log::CurrentMessageSource = msgSource;
}

CMessageSourceSwitcher::~CMessageSourceSwitcher()
{
	Log::CurrentMessageSource = prevSource;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
