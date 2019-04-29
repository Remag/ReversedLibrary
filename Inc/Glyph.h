#pragma once
#include <Redefs.h>
#include <Vector.h>

// FreeType declarations.
struct FT_GlyphRec_;
typedef struct FT_GlyphRec_* FT_Glyph;

struct FT_BitmapGlyphRec_;
typedef struct FT_BitmapGlyphRec_* FT_BitmapGlyph;

struct FT_GlyphSlotRec_;
typedef struct FT_GlyphSlotRec_* FT_GlyphSlot;

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Structure with the glyphs bitmap information.
// This structure is created to get all the necessary data in one method call.
struct CGlyphData {
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

// A single glyph from the font. Wraps FreeType structures.
// This class owns the glyph and all the bitmaps that were rendered from it.
class REAPI CGlyph {
public:
	CGlyph();
	CGlyph( CGlyph&& other );
	~CGlyph();

	CGlyph& operator=( CGlyph&& other );

	// Get UTF32 glyph code.
	int GetCode() const
		{ return glyphCode; }

	bool IsLoaded() const
		{ return bitmapGlyph != 0; }
	// Get all the data necessary to draw a glyph.
	CGlyphData GetGlyphData() const;
	// Get the bitmap buffer.
	const BYTE* GetBitmap() const;

	// Font needs access to glyph's constructor.
	friend class CFontView;

private:
	FT_BitmapGlyph bitmapGlyph;
	int glyphCode;

	// Copy a glyph from a given glyph slot.
	explicit CGlyph( FT_GlyphSlot slot, int glyphCode );

	void renderGlyph( FT_Glyph glyph );

	// Copying from another glyph is prohibited.
	CGlyph( CGlyph& ) = delete;
	void operator=( CGlyph& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.