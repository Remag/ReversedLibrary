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

CGlyph CFontView::GetGlyph( unsigned charCode, CFontSizeView fontSize ) const
{
	FT_Activate_Size( fontSize.GetHandle() );
	checkFreeTypeError( FT_Load_Char( fontFace, charCode, FT_LOAD_DEFAULT ) );
	return CGlyph( fontFace->glyph, charCode );
}

CFontSizeOwner CFontView::CreateSizeObject( int pxSize ) const
{
	return CreateSizeObject( CVector2<int>( pxSize, pxSize ) );
}

CFontSizeOwner CFontView::CreateSizeObject( CVector2<int> pxSize ) const
{
	assert( IsLoaded() );
	FT_Size newSize;
	FT_New_Size( GetFtFace(), &newSize );
	CFontSizeOwner result( newSize );
	FT_Activate_Size( newSize );
	checkFreeTypeError( FT_Set_Pixel_Sizes( GetFtFace(), pxSize.X(), pxSize.Y() ) );
	return result;
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

CFontSizeOwner CFontOwner::CreateSizeObject( int pxSize ) const
{
	return view.CreateSizeObject( pxSize );
}

CFontSizeOwner CFontOwner::CreateSizeObject( CVector2<int> pxSize ) const
{
	return view.CreateSizeObject( pxSize );
}

CGlyph CFontOwner::GetGlyph( unsigned charCode, CFontSizeView fontSize ) const
{
	return view.GetGlyph( charCode, fontSize );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

