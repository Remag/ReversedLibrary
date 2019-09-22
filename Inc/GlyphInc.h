#pragma once
#include <Redefs.h>
#include <Vector.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Structure with the glyphs bitmap information.
// This structure is created to get all the necessary data in one method call.
struct CGlyphSizeData {
	// Distance between current pen position and bitmap's top left position.
	CVector2<int> Offset;
	// An offset of the cursor after drawing the bitmap.
	CVector2<int> Advance;
	// Bitmap size in pixels.
	CVector2<int> Size;
	// Bitmap pitch. May be negative. Buffer + RowSize always gives the next row.
	int Pitch;
};

//////////////////////////////////////////////////////////////////////////

struct CGlyphData {
	CGlyphSizeData SizeData;
	// Pointer to bitmap data.
	const BYTE* BitmapData;
};

//////////////////////////////////////////////////////////////////////////

// Generic interface for receiving rasterized glyph information.
class REAPI IGlyph {
public:
	virtual ~IGlyph() {}

	virtual CGlyphData GetGlyphData() const = 0;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


