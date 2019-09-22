#pragma once
#include <GlyphInc.h>
#include <Redefs.h>
#include <Vector.h>
#include <StrConversions.h>

// FreeType declarations.
struct FT_GlyphRec_;
typedef struct FT_GlyphRec_* FT_Glyph;

struct FT_BitmapGlyphRec_;
typedef struct FT_BitmapGlyphRec_* FT_BitmapGlyph;

struct FT_GlyphSlotRec_;
typedef struct FT_GlyphSlotRec_* FT_GlyphSlot;

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A single glyph from the font. Wraps FreeType structures.
// This class owns the glyph and all the bitmaps that were rendered from it.
class REAPI CGlyph : public IGlyph {
public:
	CGlyph();
	CGlyph( CGlyph&& other );
	~CGlyph();

	CGlyph& operator=( CGlyph&& other );

	bool IsLoaded() const
		{ return bitmapGlyph != 0; }

	// IGlyph.
	virtual CGlyphData GetGlyphData() const override final;

	// Font needs access to glyph's constructor.
	friend class CFontView;

private:
	FT_BitmapGlyph bitmapGlyph;

	// Copy a glyph from a given glyph slot.
	explicit CGlyph( FT_GlyphSlot slot );

	void renderGlyph( FT_Glyph glyph );

	// Copying from another glyph is prohibited.
	CGlyph( CGlyph& ) = delete;
	void operator=( CGlyph& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.