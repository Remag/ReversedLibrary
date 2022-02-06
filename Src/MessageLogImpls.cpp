#include <MessageLogImpls.h>

#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <Reutils.h>
#include <CriticalSection.h>
#include <FileOwners.h>
#include <FileSystem.h>
#include <FileMapping.h>
#include <MessageUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

void CWindowMessageLog::AddMessage( CString text, TLogMessageType type )
{
	const UINT iconFlag = getIconFlag( type );
	::MessageBox( nullptr, UnicodeStr( text ).Ptr(), UnicodeStr( GetAppTitle() ).Ptr(), MB_OK | MB_TASKMODAL | iconFlag );
}

UINT CWindowMessageLog::getIconFlag( TLogMessageType type )
{
	staticAssert( LMT_EnumCount == 7 );
	switch( type ) {
		case LMT_Error:
		case LMT_Exception:
		case LMT_CriticalException:
			return MB_ICONSTOP;
		case LMT_Warning:
			return MB_ICONEXCLAMATION;
		case LMT_Message:
		case LMT_Periodic:
		case LMT_Success:
			return MB_ICONINFORMATION;
		default:
			assert( false );
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

CStdOutputLog::CStdOutputLog() :
	outputHandle( ::GetStdHandle( STD_OUTPUT_HANDLE ) ),
	isOutputConsole( checkConsoleHandle( outputHandle ) )
{
}

bool CStdOutputLog::checkConsoleHandle( HANDLE handle )
{
	DWORD modeDummy;
	const auto result = ::GetConsoleMode( handle, &modeDummy );
	return result != 0;
}

static CString createOutputString( CStringPart text )
{
	const int textLength = text.Length();
	CString result;
	result.ReserveBuffer( text.Length() + 2 );
	for( int i = 0; i < textLength; i++ ) {
		if( text[i] == '\n' && ( i == 0 || text[i - 1] != '\r' ) ) {
			result += '\r';
		}
		result += text[i];
	}

	result += "\r\n";
	return result;	
}

extern CCriticalSection ConsoleWriteSection;
void CStdOutputLog::AddMessage( CString text, TLogMessageType )
{
	CCriticalSectionLock lock( ConsoleWriteSection );
	const auto outputStr = createOutputString( text );
	DWORD writeCount;
	if( isOutputConsole ) {
		::WriteConsoleA( outputHandle, outputStr.Ptr(), outputStr.Length(), &writeCount, nullptr );
	} else {
		::WriteFile( outputHandle, outputStr.Ptr(), outputStr.Length(), &writeCount, nullptr );
	}
}

//////////////////////////////////////////////////////////////////////////

extern CCriticalSection FileWriteSection;
CFileMessageLog::CFileMessageLog( CStringPart _fileName, int targetFileSize ) :
	currentFileName( _fileName )
{
	const auto maxSize = max( 32, targetFileSize );
	
	CCriticalSectionLock lock( FileWriteSection );
	CFileReadWriter file( currentFileName, FCM_CreateOrOpen );
	const auto length = file.GetLength32();
	if( length >= maxSize ) {
		const auto newFileSize = FloorTo( maxSize / 2, sizeof( wchar_t ) );
		moveRecentLogToStart( _fileName, length, newFileSize );
		file.SetLength( newFileSize );
	}
}

void CFileMessageLog::moveRecentLogToStart( CStringPart fileName, int oldFileSize, int newFileSize )
{
	assert( newFileSize > 2 );
	assert( newFileSize <= oldFileSize );
	CFileMapping logMapping( fileName, CFileMapping::MM_ReadWrite );
	auto view = logMapping.CreateReadWriteView( 0, oldFileSize );
	assert( view.Size() >= newFileSize );
	const auto cutFileSize = view.Size() - newFileSize;
	const auto buffer = view.GetBuffer();
	::memcpy( buffer, buffer + cutFileSize, newFileSize );
	const CUnicodeView newLineBuffer = L"\r\n";
	const auto newLineBufferSize = sizeof( wchar_t ) * newLineBuffer.Length();
	::memcpy( buffer + cutFileSize - newLineBufferSize, newLineBuffer.Ptr(), newLineBufferSize );
}

void CFileMessageLog::AddMessage( CString text, TLogMessageType )
{
	CCriticalSectionLock lock( FileWriteSection );
	CFileWriter file( currentFileName, FCM_CreateOrOpen );

	const auto filePos = file.SeekToEnd();
	if( filePos == 0 ) {
		// File is empty, make it unicode.
		initializeUnicodeFile( file );
	}
	writeToFile( file, text );
}

namespace RelibInternal {
	extern const char Utf16LEFileTag[2];
}
void CFileMessageLog::initializeUnicodeFile( CFileWriteView target )
{
	target.Write( RelibInternal::Utf16LEFileTag, sizeof( RelibInternal::Utf16LEFileTag ) );
}

void CFileMessageLog::writeToFile( CFileWriteView target, CStringPart text )
{
	const auto rawOutput = createOutputString( text );
	const auto output = addUtilityInfo( rawOutput );
	target.Write( output.Ptr(), output.Length() * sizeof( wchar_t ) );
}

CString CFileMessageLog::addUtilityInfo( CStringPart text ) const
{
	const auto dateStr = Str( CDateTime::Now(), "[YYYY.MM.DD H:M:S] " );
	return dateStr + text;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
