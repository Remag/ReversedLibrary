#pragma once
#include <MessageLog.h>
#include <BaseString.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Message logger that outputs messages in a dialog window.
class REAPI CWindowMessageLog : public IMessageLog {
public:
	virtual void AddMessage( CUnicodeView text, TLogMessageType type, CMessageSource src ) override final;

private:
	static UINT getIconFlag( TLogMessageType type );
};

// Message logger that outputs messages in a standard output.
// File and console standard outputs are supported.
class REAPI CStdOutputLog : public IMessageLog {
public:
	CStdOutputLog();

	// Return true if standard output redirects to a console.
	bool IsStdOutputConsole() const
		{ return isOutputConsole; }

	virtual void AddMessage( CUnicodeView text, TLogMessageType type, CMessageSource src ) override final;

private:
	HANDLE outputHandle;
	bool isOutputConsole;

	static bool checkConsoleHandle( HANDLE handle );
};

// Message logger that outputs messages in a rotating file.
// Two log files are created with the given name and a unique suffix: "filename_1.log" and "filename_2.log".
// When the target file size is reached for the current file, the target file is switched.
class REAPI CFileMessageLog : public IMessageLog {
public:
	explicit CFileMessageLog( CUnicodeView _fileName, int targetFileSize ); 

	virtual void AddMessage( CUnicodeView text, TLogMessageType type, CMessageSource src ) override final;

private:
	CUnicodeString currentFileName;
	CUnicodeString backFileName;
	int maxSize;

	void initializeUnicodeFile( CFileWriteView target );
	void writeToFile( CFileWriteView target, CUnicodeView text );
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

