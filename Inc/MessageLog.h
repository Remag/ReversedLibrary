#pragma once
#include <Redefs.h>
#include <BaseString.h>
#include <BaseStringView.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// General type of a log message.
enum TLogMessageType {
	LMT_Success,
	LMT_Message,
	LMT_Warning,
	LMT_Error,
	LMT_Exception,
	LMT_CriticalException,
	LMT_Periodic,
	LMT_EnumCount
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for displaying a given text message.
class REAPI IMessageLog {
public:
	IMessageLog() = default;
	virtual ~IMessageLog() {}

	virtual void AddMessage( CString text, TLogMessageType type ) = 0;

private:
	// Copying is prohibited.
	IMessageLog( IMessageLog& ) = delete;
	void operator=( IMessageLog& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Current message log switcher.
// Each thread has its own message log.
class REAPI CMessageLogSwitcher {
public:
	explicit CMessageLogSwitcher( IMessageLog& newValue );
	~CMessageLogSwitcher();

private:
	IMessageLog* prevLog;

	// Copying is prohibited.
	CMessageLogSwitcher( const CMessageLogSwitcher& ) = delete;
	void operator=( const CMessageLogSwitcher& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Message source switcher. A message source is a string value indicating a currently operating subsystem.
// Each thread has its own message source.
class REAPI CMessageSourceSwitcher {
public:
	explicit CMessageSourceSwitcher( CStringPart srcName );
	~CMessageSourceSwitcher();

private:
	CString msgSource;
	CStringView prevSource;

	// Copying is prohibited.
	CMessageSourceSwitcher( const CMessageSourceSwitcher& ) = delete;
	void operator=( const CMessageSourceSwitcher& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

void REAPI sendLogMessage( TLogMessageType msgType, CString text );

template <class T>
CString createMessageLogString( const T& lastMsg )
{
	return Str( lastMsg );
}

template <class T, class... TT>
CString createMessageLogString( const T& msg, const TT&... rest )
{
	return Str( msg ) + ' ' + createMessageLogString( rest... );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// General logging functions.
namespace Log {
	
// Display a thrown exception. Exceptions are treated as errors.
void REAPI Exception( const CException& e );
// Display an unhangled exception. Should be used in cases where the system cannot recover.
void REAPI CriticalException( const CException& e );

// Display an error message. Error messages should be used when further behavior of the program is unpredictable.
// Errors cannot be filtered out.
template <class... TT>
void Error( const TT&... messages )
{
	RelibInternal::sendLogMessage( LMT_Error, RelibInternal::createMessageLogString( messages... ) );
}

// Display a warning.
template <class... TT>
void Warning( const TT&... messages )
{
	RelibInternal::sendLogMessage( LMT_Warning, RelibInternal::createMessageLogString( messages... ) );
}

// Display a regular message.
template <class... TT>
void Message( const TT&... messages )
{
	RelibInternal::sendLogMessage( LMT_Message, RelibInternal::createMessageLogString( messages... ) );
}

// Display a message indicating a successful operation.
template <class... TT>
void Success( const TT&... messages )
{
	RelibInternal::sendLogMessage( LMT_Success, RelibInternal::createMessageLogString( messages... ) );
}

// Send a periodic message with new data.
template <class... TT>
void PeriodicUpdate( const TT&... messages )
{
	RelibInternal::sendLogMessage( LMT_Periodic, RelibInternal::createMessageLogString( messages... ) );
}

CStringView REAPI GetCurrentMessageSource();

}	// namespace Log.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

