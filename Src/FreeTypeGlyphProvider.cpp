#include <FreeTypeGlyphProvider.h>
#include <Glyph.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CFreeTypeGlyphProvider::CFreeTypeGlyphProvider( CFontView font, int fontPxHeight ) :
	fontView( font ),
	fontSize( font.CreateSizeObject( fontPxHeight ) )
{
}

unsigned CFreeTypeGlyphProvider::GetGlyphIndex( int utf32 ) const
{
	return fontView.GetGlyphIndex( utf32 );
}

CPtrOwner<IGlyph> CFreeTypeGlyphProvider::GetGlyphByIndex( unsigned index ) const
{
	return CreateOwner<CGlyph>( fontView.GetGlyphByIndex( index, fontSize ) );
}

CPtrOwner<IGlyph> CFreeTypeGlyphProvider::GetGlyph( int utf32 ) const
{
	return CreateOwner<CGlyph>( fontView.GetGlyph( utf32, fontSize ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
