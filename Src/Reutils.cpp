#include <Reutils.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <CriticalSection.h>
#include <RelibInitializer.h>
#include <StrConversions.h>
#include <Array.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

static int parseSingleArgument( int pos, CUnicodeView commandLine, CArray<CUnicodePart>& result )
{
	const auto length = commandLine.Length();
	while( CUnicodeString::IsCharWhiteSpace( commandLine[pos] ) ) {
		pos++;
	}
	wchar_t stopSymbol;
	int startPos;
	if( length > pos && commandLine[pos] == L'"' ) {
		startPos = pos + 1;
		stopSymbol = L'"';
	} else {
		startPos = pos;
		stopSymbol = L' ';
	}

	int stopPos = commandLine.Find( stopSymbol, startPos );
	CUnicodePart fullArg;
	if( stopPos != NotFound ) {
		fullArg = commandLine.Mid( startPos, stopPos - startPos );
		stopPos++;
	} else {
		fullArg = commandLine.Mid( startPos );
		stopPos = length - startPos;
	}

	result.Add( fullArg.TrimSpaces() );
	return stopPos;
}

CArray<CUnicodePart> SplitCommandLine( CUnicodeView commandLine )
{
	CArray<CUnicodePart> result;
	int pos = 0;
	while( pos < commandLine.Length() ) {
		const int newPos = parseSingleArgument( pos, commandLine, result );
		assert( newPos > pos );
		pos = newPos;
	}

	return result;
}

static TLibraryDebugMode currentRelibDebugMode = LDM_DebugStop;
TLibraryDebugMode GetLibraryDebugMode()
{
	return currentRelibDebugMode;
}

void SetLibraryDebugMode( TLibraryDebugMode newMode )
{
	currentRelibDebugMode = newMode;
}

static bool isSilentAbort = false;
bool IsSilentAbort()
{
	return isSilentAbort;
}

void SetSilentAbort( bool value )
{
	isSilentAbort = value;
}

static DWORD debugFlags = DF_ShowInternalErrorMessage;
DWORD GetDebugFlags()
{
	return debugFlags;
}

void SetDebugFlags( DWORD newFlags )
{
	debugFlags = newFlags;
}

extern CUnicodeString ApplicationTitle;
extern CCriticalSection ApplicationTitleSection;
CUnicodeView GetAppTitle()
{
	CCriticalSectionLock lock( ApplicationTitleSection );
	return ApplicationTitle;
}

void SetAppTitle( CUnicodeView newTitle )
{
	CCriticalSectionLock lock( ApplicationTitleSection );
	ApplicationTitle = newTitle;
}

bool IsRelibInitialized()
{
	return CRelibInitializer::GetInstance() != nullptr;
}

static SYSTEM_INFO createSystemInfo()
{
	SYSTEM_INFO result;
	::GetSystemInfo( &result );
	return result;
}

int GetProcessorCount()
{
	static SYSTEM_INFO info = createSystemInfo();
	return info.dwNumberOfProcessors;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
