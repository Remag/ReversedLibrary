#include <Font.h>
#include <FontSize.h>
#include <Glyph.h>
#include <FreeTypeException.h>
#include <FileOwners.h>
#include <UnicodeUtils.h>
#include <StrConversions.h>
#include <MessageLog.h>

#include <FreeType\ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CFontView::CFontView( const CFontOwner& owner ) :
	CFontView( owner.View() )
{
}

CGlyph CFontView::GetGlyph( int charCode ) const
{
	checkFreeTypeError( FT_Load_Char( fontFace, charCode, FT_LOAD_DEFAULT ) );
	return CGlyph( fontFace->glyph, charCode );
}

static const int basicAsciiBegin = 32;
static const int basicAsciiEnd = 128;
bool CFontView::GetGlyph( char character, CGlyph& result ) const
{
	if( character < basicAsciiBegin || character >= basicAsciiEnd ) {
		return false;
	}

	// ASCII encoded characters are the same in UTF32 encoding.
	result = GetGlyph( numeric_cast<int>( character ) );
	return true;
}

bool CFontView::GetUnicodeGlyph( wchar_t charCode, CGlyph& result ) const
{
	int resultCode;
	const bool canConvert = Unicode::TryConvertWideToInt( charCode, resultCode );
	if( !canConvert ) {
		return false;
	} 
	result = GetGlyph( resultCode );
	return true;
}

bool CFontView::GetUnicodeGlyph( wchar_t hiCode, wchar_t loCode, CGlyph& result ) const
{
	int resultCode;
	const bool canConvert = Unicode::TryConvertWideToInt( hiCode, loCode, resultCode );
	if( !canConvert ) {
		return false;
	} 
	result = GetGlyph( resultCode );
	return true;
}

//////////////////////////////////////////////////////////////////////////

void CFontEdit::SetPixelSize( CVector2<int> size )
{
	assert( IsLoaded() );
	checkFreeTypeError( FT_Set_Pixel_Sizes( GetFtFace(), size.X(), size.Y() ) );
}

void CFontEdit::SetPixelSize( int size )
{
	SetPixelSize( CVector2<int>( size, size ) );
}

void CFontEdit::SetPointSize( CVector2<float> size )
{
	assert( IsLoaded() );
	const auto dpi = 96;
	checkFreeTypeError( FT_Set_Char_Size( GetFtFace(), Round( 64 * size.X() ), Round( 64 * size.Y() ), dpi, dpi ) );
}

void CFontEdit::SetPointSize( float size )
{
	SetPointSize( CVector2<float>( size, size ) );
}

CFontSizeOwner CFontEdit::CreateSizeObject( float ptSize )
{
	return CreateSizeObject( CVector2<float>( ptSize, ptSize ) );
}

CFontSizeOwner CFontEdit::CreateSizeObject( CVector2<float> ptSize )
{
	assert( IsLoaded() );
	FT_Size newSize;
	FT_New_Size( GetFtFace(), &newSize );
	FT_Activate_Size( newSize );
	SetPointSize( ptSize );
	return CFontSizeOwner( newSize );
}

void CFontEdit::ActivateSize( CFontSizeView size )
{
	FT_Activate_Size( size.GetHandle() );
}

//////////////////////////////////////////////////////////////////////////

CFontOwner::CFontOwner( CUnicodeView name )
{
	Load( name );
}

CFontOwner::CFontOwner( CFontOwner&& other ) :
	fontData( move( other.fontData ) ),
	view( other.view )
{
	other.detachView();
}

CFontOwner& CFontOwner::operator=( CFontOwner&& other )
{
	if( IsLoaded() ) {
		Unload();
	}

	fontData = move( other.fontData );
	view = move( other.view );

	other.detachView();
	return *this;
}

void CFontOwner::detachView()
{
	view.GetFtFace() = nullptr;
}

CFontOwner::~CFontOwner()
{
	try {
		if( IsLoaded() ) {
			Unload();
		}
	} catch( const CException& e ) {
		Log::Exception( e );
		cleanup();
	}
	assert( !IsLoaded() );
}

// Clear Relib internal structures.
void CFontOwner::cleanup()
{
	fontData.FreeBuffer();
}

void CFontOwner::Load( CUnicodeView name )
{
	assert( !IsLoaded() );

	// Open the file.
	CFileReader fontFile( name, FCM_OpenExisting );

	// Get file contents in the buffer.
	CArray<BYTE> data;
	data.IncreaseSizeNoInitialize( fontFile.GetLength32() );
	fontFile.Read( data.Ptr(), data.Size() );

	// Create a font face from contents.
	checkFreeTypeError( FT_New_Memory_Face( freeTypeLib, data.Ptr(), data.Size(), 0, &view.GetFtFace() ) );

	// No exceptions were thrown, fill Relib structures.
	fontData = move( data );
}

void CFontOwner::Unload()
{
	assert( IsLoaded() );

	checkFreeTypeError( FT_Done_Face( view.GetFtFace() ) );
	cleanup();
	detachView();
}

void CFontOwner::SetPixelSize( CVector2<int> size )
{
	view.SetPixelSize( size );
}

void CFontOwner::SetPixelSize( int size )
{
	view.SetPixelSize( size );
}

void CFontOwner::SetPointSize( CVector2<float> size )
{
	view.SetPointSize( size );
}

void CFontOwner::SetPointSize( float size )
{
	view.SetPointSize( size );
}

CGlyph CFontOwner::GetGlyph( int charCode ) const
{
	return view.GetGlyph( charCode );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

