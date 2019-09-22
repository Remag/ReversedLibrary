#include <Glyph.h>
#include <FreeTypeException.h>
#include <BaseStringPart.h>
#include <StrConversions.h>

#include <FreeType\ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CGlyph::CGlyph() :
	bitmapGlyph( 0 )
{
}

CGlyph::CGlyph( FT_GlyphSlot slot )
{
	FT_Glyph glyph;
	checkFreeTypeError( FT_Get_Glyph( slot, &glyph ) );
	renderGlyph( glyph );
}

CGlyph::CGlyph( CGlyph&& other ) :
	bitmapGlyph( other.bitmapGlyph )
{
	other.bitmapGlyph = 0;
}

CGlyph::~CGlyph()
{
	FT_Done_Glyph( reinterpret_cast<FT_Glyph>( bitmapGlyph ) );
}

CGlyph& CGlyph::operator=( CGlyph&& other )
{
	bitmapGlyph = other.bitmapGlyph;
	other.bitmapGlyph = 0;
	return *this;
}

CGlyphData CGlyph::GetGlyphData() const
{
	CGlyphData result;
	result.SizeData.Offset.X() = bitmapGlyph->left;
	result.SizeData.Offset.Y() = bitmapGlyph->top;
	result.SizeData.Size.X() = bitmapGlyph->bitmap.width;
	result.SizeData.Size.Y() = bitmapGlyph->bitmap.rows;
	result.SizeData.Pitch = bitmapGlyph->bitmap.pitch;
	result.SizeData.Advance.X() = bitmapGlyph->root.advance.x >> 16;
	result.SizeData.Advance.Y() = bitmapGlyph->root.advance.y >> 16;
	result.BitmapData = bitmapGlyph->bitmap.buffer;
	return result;
}

void CGlyph::renderGlyph( FT_Glyph glyph )
{
	checkFreeTypeError( FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 ) );
	bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>( glyph );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

