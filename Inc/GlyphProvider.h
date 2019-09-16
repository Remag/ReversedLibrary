#pragma once
#include <Redefs.h>
#include <PtrOwner.h>
#include <GlyphInc.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Generic interface for rendering a glyph.
class REAPI IGlyphProvider {
public:
	virtual ~IGlyphProvider() {}

	virtual CPtrOwner<IGlyph> GetGlyph( int utf32 ) const = 0;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


