#pragma once
// Functions to control library behavior.

#include <Redefs.h>
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// AddressOf implementation for function pointers.
template <class T>
T* addressOfImpl( T& value, Types::TrueType )
{
	return value;
}

template <class T>
T* addressOfImpl( T& value, Types::FalseType )
{
	return reinterpret_cast<T*>( &const_cast<char&>( reinterpret_cast<const volatile char&>( value ) ) );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Get a pointer to a given variable. Doesn't trigger operator&.
template <class T>
T* AddressOf( T& value )
{
	return RelibInternal::addressOfImpl( value, Types::IsFunction<T>() );
}

// Split a single string command line into a list of command line arguments.
CArray<CUnicodePart> REAPI SplitCommandLine( CUnicodeView commandLine );

// Library behavior when internal errors occur.
enum TLibraryDebugMode {
	LDM_DontStop,	// Don't break on errors.
	LDM_DebugStop,	// Break if debugger is present.
	LDM_AlwaysStop	// Always break.
};

TLibraryDebugMode REAPI GetLibraryDebugMode();
void REAPI SetLibraryDebugMode( TLibraryDebugMode newMode );

enum TDebugFlag {
	DF_ShowInternalErrorMessage	= 0x001,	// Show an additional message on Internal Program Errors.
	DF_ReportMemoryErrorsInMessageBox = 0x002,	// Use MessageBox to report memory leaks.
};

// Setting/getting current debug flags.
DWORD REAPI GetDebugFlags();
void REAPI SetDebugFlags( DWORD newFlags );

// Should the library show message boxes before calling abort.
// Currently checked on memory exceptions.
bool REAPI IsSilentAbort();
void REAPI SetSilentAbort( bool value );

CUnicodeView REAPI GetAppTitle();
void REAPI SetAppTitle( CUnicodeView newTitle );

// Check library initialization.
bool REAPI IsRelibInitialized();

// System information.
// Number of logical processors.
int REAPI GetProcessorCount();

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

