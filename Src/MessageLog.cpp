#include <MessageLog.h>
#include <MessageUtils.h>
#include <BaseString.h>
#include <BaseStringView.h>
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

//////////////////////////////////////////////////////////////////////////

void Error( CUnicodeView text, CMessageSource subsystem )
{
	getCurrentMessageLog().AddMessage( text, LMT_Error, subsystem );
}

void Exception( const CException& e )
{
	struct CExceptionMessageTag{};
	getCurrentMessageLog().AddMessage( e.GetMessageText(), LMT_Exception, CExceptionMessageTag{} );
}

void Warning( CUnicodeView text, CMessageSource subsystem )
{
	if( IsMessageSourceShown( subsystem ) ) {
		getCurrentMessageLog().AddMessage( text, LMT_Warning, subsystem );
	}
}

void Message( CUnicodeView text, CMessageSource subsystem )
{
	if( IsMessageSourceShown( subsystem ) ) {
		getCurrentMessageLog().AddMessage( text, LMT_Message, subsystem );
	}
}

void PeriodicUpdate( CUnicodeView text, CMessageSource subsystem )
{
	if( IsMessageSourceShown( subsystem ) ) {
		getCurrentMessageLog().AddMessage( text, LMT_Periodic, subsystem );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Log.

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

}	// namespace Relib.
