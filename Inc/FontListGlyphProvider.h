#pragma once
#include <GlyphProvider.h>
#include <FreeTypeGlyphProvider.h>
#include <Array.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Provider that tries to get glyphs from a given font list until a valid glyph is found.
class REAPI CFontListGlyphProvider : public IGlyphProvider {
public:
	void AddFont( CFontView fontView, int fontPxHeight );
	// IGlyphProvider.
	virtual CPtrOwner<IGlyph> GetGlyph( int utf32 ) const override final;

private:
	CArray<CFreeTypeGlyphProvider> fontList;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


