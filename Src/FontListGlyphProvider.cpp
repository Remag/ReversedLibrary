#include <FontListGlyphProvider.h>
#include <Glyph.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

void CFontListGlyphProvider::AddFont( CFontView fontView, int fontPxHeight )
{
	fontList.Add( fontView, fontPxHeight );
}

CPtrOwner<IGlyph> CFontListGlyphProvider::GetGlyph( int utf32 ) const
{
	assert( !fontList.IsEmpty() );
	for( const auto& font : fontList ) {
		const auto glyphIndex = font.GetGlyphIndex( utf32 );
		if( glyphIndex != 0 ) {
			return font.GetGlyphByIndex( glyphIndex );
		}
	}
	return fontList[0].GetGlyph( utf32 );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
