#pragma once
#include <Reutils.h>

namespace Relib {

// Generate the Internal Program Error.
void REAPI GenerateInternalError( const char* functionName, const wchar_t* errorText, const wchar_t* fileName, int line );

// Stop the program execution and prompt user to debug.
inline void ProgramBreakPoint()
{
	if( GetLibraryDebugMode() != LDM_DontStop ) {
		__debugbreak();
	}
}

// This function breaks only if a debugger is present.
// Function is defined as a macro so that execution would stop at the correct point.
#define ProgramDebuggerBreak() if( IsDebuggerPresent() != 0 && GetLibraryDebugMode() != LDM_DontStop ) { __debugbreak(); } else {}
// This function breaks only in Debug mode with the debugger present.
#ifdef _DEBUG
#define ProgramDebugModeBreak() ProgramDebuggerBreak()
#else
#define ProgramDebugModeBreak()
#endif

// Assert macro.
#if !defined( RELIB_FINAL )
#define assert( expr ) \
if( !( expr ) ) { \
	ProgramDebugModeBreak()	\
	Relib::GenerateInternalError( __FUNCTION__, L###expr, __UNICODEFILE__, __LINE__ ); \
} else {}
#else
#define assert( expr ) ( ( void ) 0 )
#endif

#ifdef DEBUG
#define debug_assert( expr ) assert( ( expr ) )

#else
#define debug_assert( expr ) ( ( void ) 0 )
#endif


#define staticAssert( condition ) static_assert( ( condition ), "Compile time assertion failed." )

}	// namespace Relib.