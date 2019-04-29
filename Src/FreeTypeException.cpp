#include <FreeTypeException.h>
#include <StrConversions.h>

#include <FreeType\ft2build.h>
#include FT_FREETYPE_H

namespace Relib {

extern const CUnicodeView GeneralFreeTypeError;
//////////////////////////////////////////////////////////////////////////

CFreeTypeException::CFreeTypeException( int code ) :
	errorBase( GeneralFreeTypeError )
{
	setInfoFromCode( code );
}

// Code contains information about the error and the module that threw an error.
// This method gets this information from the code.
void CFreeTypeException::setInfoFromCode( int code )
{
	errorCode = FT_ERROR_BASE( code );
	const int moduleCode = FT_ERROR_MODULE( code );
	moduleName = getModuleName( moduleCode );
}

CUnicodeView CFreeTypeException::getModuleName( int moduleCode )
{
#ifdef FT_CONFIG_OPTION_USE_MODULE_ERRORS
	switch( moduleCode ) {
	case FT_Mod_Err_Base:
		return L"Base";
	case FT_Mod_Err_Autofit:
		return L"Autofit";
	case FT_Mod_Err_BDF:
		return L"BDF";
	case FT_Mod_Err_Bzip2:
		return L"Bzip2";
	case FT_Mod_Err_Cache:
		return L"Cache";
	case FT_Mod_Err_CFF:
		return L"CFF";
	case FT_Mod_Err_CID:
		return L"CID";
	case FT_Mod_Err_LZW:
		return L"LZW";
	case FT_Mod_Err_OTvalid:
		return L"OTvalid";
	case FT_Mod_Err_PCF:
		return L"PCF";
	case FT_Mod_Err_PFR:
		return L"PFR";
	case FT_Mod_Err_PSaux:
		return L"PSaux";
	case FT_Mod_Err_PShinter:
		return L"PShinter";
	case FT_Mod_Err_PSnames:
		return L"PSnames";
	case FT_Mod_Err_Raster:
		return L"Raster";
	case FT_Mod_Err_SFNT:
		return L"SFNT";
	case FT_Mod_Err_Smooth:
		return L"Smooth";
	case FT_Mod_Err_TrueType:
		return L"TrueType";
	case FT_Mod_Err_Type1:
		return L"Type1";
	case FT_Mod_Err_Type42:
		return L"Type42";
	case FT_Mod_Err_Winfonts:
		return L"Winfonts";
	case FT_Mod_Err_GXvalid:
		return L"GXvalid";
	default:
		return L"Unknown";
	}
#else
	moduleCode;
	return L"Unknown";
#endif
}

CUnicodeString CFreeTypeException::GetMessageText() const
{
	return errorBase.SubstParam( UnicodeStr( errorCode ), moduleName );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.