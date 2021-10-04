#include <RelibInitializer.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <FileSystem.h>

#include <Errors.h>
#include <Font.h>
#include <FreeTypeException.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CRelibInitializer::CRelibInitializer()
{
	// Turn on memory leak detection.
	_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | _CRTDBG_LEAK_CHECK_DF );
	// Initialize the program path to Application Data folder.
	initAppDataPath();
	initFreeType();
}

CRelibInitializer::~CRelibInitializer()
{
	assert( CFontOwner::freeTypeLib != 0 );

	checkFreeTypeError( FT_Done_FreeType( CFontOwner::freeTypeLib ) );
	CFontOwner::freeTypeLib = 0;
}

void CRelibInitializer::initFreeType()
{
	assert( CFontOwner::freeTypeLib == 0 );

	checkFreeTypeError( FT_Init_FreeType( &CFontOwner::freeTypeLib ) );
}

void CRelibInitializer::initAppDataPath()
{
	const CUnicodeString processName = FileSystem::GetExecutableName();
	try {
		const CUnicodeString relativePath = L"Relib\\" + FileSystem::GetName( processName );
		FileSystem::SetAppDataRelativePath( relativePath );
		return;
	} catch( const CException& ) {
		// Getting the application data path is impossible for some reason.
		// Use the executable path.
		FileSystem::SetAppDataPath( FileSystem::ADPT_AllUsers, processName );
		FileSystem::SetAppDataPath( FileSystem::ADPT_SpecificUser, processName );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
