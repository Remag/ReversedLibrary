#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// User process wrapper.
class REAPI CProcess {
public:
	CProcess();
	explicit CProcess( CUnicodeString commandLine );
	~CProcess();

	// Process info data access.
	HANDLE ProcessHandle() const
		{ return procInfo.hProcess; }
	DWORD ProcessId() const
		{ return procInfo.dwProcessId; }
	HANDLE ThreadHandle() const
		{ return procInfo.hThread; }
	DWORD ThreadId() const
		{ return procInfo.dwThreadId; }

	bool IsValid() const 
		{ return procInfo.hProcess != 0; }

	// Create process with the given command line.
	void Create( CUnicodeString commandLine );
	// Wait for a process to finish.
	// Returns whether process was able to finish within the given timeout.
	bool Wait( DWORD waitTimeout = INFINITE );
	// Get and exit code of a finished process.
	DWORD ExitCode() const;
	// Close the process handle.
	void Abandon();
	// Terminate the process with the given exit code.
	void Terminate( DWORD exitCode = 0 );

	// Create a process and close its handle upon creation.
	static void CreateAndAbandon( CUnicodeString commandLine );
	// Create a process and wait for its completion.
	// Returns whether process was able to finish within the given timeout.
	static bool CreateAndWait( CUnicodeString commandLine, DWORD waitTimeout = INFINITE );

private:
	// System structure that is returned by CreateProcess.
	PROCESS_INFORMATION procInfo;

	void closeHandles();
	void initProcInfo();

	// Copying is prohibited.
	CProcess( const CProcess& ) = delete;
	void operator=( const CProcess& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

