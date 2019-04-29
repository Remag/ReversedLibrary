#include <ReProcess.h>

#include <Reassert.h>
#include <Errors.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CProcess::CProcess()
{
	initProcInfo();
}

CProcess::CProcess( CUnicodeString commandLine )
{
	initProcInfo();
	Create( move( commandLine ) );
}

CProcess::~CProcess()
{
	if( procInfo.hProcess != 0 ) {
		CloseHandle( procInfo.hProcess );
	}
	if( procInfo.hThread != 0 ) {
		CloseHandle( procInfo.hThread );
	}
}

void CProcess::Create( CUnicodeString commandLine )
{
	assert( !IsValid() );

	STARTUPINFO info;
	ZeroMemory( &info, sizeof( info ) );
	info.cb = sizeof( info );
	
	checkLastError( CreateProcess( 0, commandLine.CreateRawBuffer(), 0, 0, FALSE, 0, 0, 0, &info, &procInfo ) != FALSE );
}

bool CProcess::Wait( DWORD waitTimeout /*= INFINITE */ )
{
	assert( IsValid() );
	
	const DWORD waitResult = WaitForSingleObject( procInfo.hProcess, waitTimeout );
	switch( waitResult ) {
	case WAIT_OBJECT_0:
		closeHandles();
		return true;
	case WAIT_TIMEOUT:
		return false;
	default:
		checkLastError( false );
		return false;
	}
}

DWORD CProcess::ExitCode() const
{
	assert( IsValid() );

	DWORD result;
	GetExitCodeProcess( procInfo.hProcess, &result );
	return result;
}

void CProcess::Abandon()
{
	assert( IsValid() );
	closeHandles();
}

void CProcess::Terminate( DWORD exitCode /*= 0 */ )
{
	assert( IsValid() );

	SetLastError( NOERROR );
	TerminateProcess( procInfo.hProcess, exitCode );
	const DWORD terminateError = GetLastError();

	closeHandles();
	if( terminateError != NOERROR ) {
		GenerateLastErrorException( terminateError );
	}
}

void CProcess::CreateAndAbandon( CUnicodeString commandLine )
{
	CProcess process( move( commandLine ) );
}

bool CProcess::CreateAndWait( CUnicodeString commandLine, DWORD waitTimeout /*= INFINITE */ )
{
	CProcess process( move( commandLine ) );
	return process.Wait( waitTimeout );
}

void CProcess::closeHandles()
{
	SetLastError( NOERROR );
	CloseHandle( procInfo.hProcess );
	const DWORD procError = GetLastError();
	CloseHandle( procInfo.hThread );
	const DWORD threadError = GetLastError();
	initProcInfo();

	if( procError != NOERROR ) {
		GenerateLastErrorException( procError );
	}
	if( threadError != NOERROR ) {
		GenerateLastErrorException( threadError );
	}
}

void CProcess::initProcInfo()
{
	ZeroMemory( &procInfo, sizeof( procInfo ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.