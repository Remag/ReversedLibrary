#pragma once
#include <Redefs.h>
#include <GlyphProvider.h>
#include <Font.h>
#include <FontSize.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Glyph provider that utilizes FreeType to render requested glyphs.
class REAPI CFreeTypeGlyphProvider : public IGlyphProvider {
public:
	CFreeTypeGlyphProvider( CFontView font, int fontPxHeight );

	unsigned GetGlyphIndex( int utf32 ) const;
	CPtrOwner<IGlyph> GetGlyphByIndex( unsigned index ) const;

	virtual CPtrOwner<IGlyph> GetGlyph( int utf32 ) const override final;

private:
	CFontView fontView;
	CFontSizeOwner fontSize;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


