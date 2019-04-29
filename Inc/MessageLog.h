#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// General type of a log message.
enum TLogMessageType {
	LMT_Message,
	LMT_Warning,
	LMT_Error,
	LMT_Exception,
	LMT_Periodic,
	LMT_EnumCount
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for displaying a given text message.
class REAPI IMessageLog {
public:
	IMessageLog() = default;
	virtual ~IMessageLog() {}

	virtual void AddMessage( CUnicodeView text, TLogMessageType type, CMessageSource src ) = 0;

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
};

//////////////////////////////////////////////////////////////////////////

// General logging functions.
namespace Log {
	
// Display an error message. Error messages should be used when further behavior of the program is unpredictable.
// Errors cannot be filtered out.
void REAPI Error( CUnicodeView text, CMessageSource subsystem );
// Display an thrown exception. Exceptions are treated as errors.
void REAPI Exception( const CException& e );

// Display a warning. Warnings indicate that a given subsystem does not behave in an intended way.
void REAPI Warning( CUnicodeView text, CMessageSource subsystem );

// Display a regular message.
void REAPI Message( CUnicodeView text, CMessageSource subsystem );

// Send a periodic message with new data.
void REAPI PeriodicUpdate( CUnicodeView text, CMessageSource subsystem );

}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

