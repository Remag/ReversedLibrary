#pragma once
#include <Reutils.h>
#include <Array.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <Vector.h>

struct FT_Library_Rec_;
typedef struct FT_LibraryRec_* FT_Library;

struct FT_Face_Rec_;
typedef struct FT_FaceRec_* FT_Face;

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A non owning font view.
class REAPI CFontView {
public:
	CFontView() = default;
	explicit CFontView( FT_Face face ) : fontFace( face ) {}
	explicit CFontView( const CFontOwner& owner );

	bool IsLoaded() const
		{ return fontFace != nullptr; }

	FT_Face GetFtFace() const
		{ return fontFace; }

	// Get the glyph structure for the given character code in UTF32BE encoding.
	CGlyph GetGlyph( int charCode ) const;

	// Get the glyph structure for the given character.
	// Only basic ASCII characters are supported.
	// If an non-basic ASCII character is passed, false is returned.
	bool GetGlyph( char character, CGlyph& result ) const;
	// Support for wchar_t.
	// Get a glyph of a simple UTF16 symbol.
	// Is the symbol has no translation, return false.
	bool GetUnicodeGlyph( wchar_t charCode, CGlyph& result ) const;
	// Get a glyph of a UTF16 symbol that consists of two character codes.
	// Is the codes have no translation, return false.
	bool GetUnicodeGlyph( wchar_t hiCode, wchar_t loCode, CGlyph& result ) const;

protected:
	// FreeType internal structure.
	FT_Face fontFace = nullptr;
};

//////////////////////////////////////////////////////////////////////////

// A non-owning font object that provides non-constant operations.
class REAPI CFontEdit : public CFontView {
public:
	FT_Face& GetFtFace()
		{ return fontFace; }

	// Set font size in pixels.
	void SetPixelSize( CVector2<int> size );
	void SetPixelSize( int size );
	// Set font size in points. 96 dpi is assumed.
	void SetPointSize( CVector2<float> size );
	void SetPointSize( float size );

	// Create an object with size information. It is activated on creation.
	CFontSizeOwner CreateSizeObject( float ptSize );
	CFontSizeOwner CreateSizeObject( CVector2<float> ptSize );
	void ActivateSize( CFontSizeView size );
};

//////////////////////////////////////////////////////////////////////////

// FreeType owning font wrapper.
class REAPI CFontOwner {
public:
	CFontOwner() = default;
	explicit CFontOwner( CUnicodeView fileName );
	CFontOwner( CFontOwner&& other );
	CFontOwner& operator=( CFontOwner&& other );
	~CFontOwner();

	CFontView View() const
		{ return view; }
	operator CFontEdit()
		{ return view; }
	operator CFontView() const
		{ return view; }

	bool IsLoaded() const
		{ return view.IsLoaded(); }
	void Load( CUnicodeView fileName );
	void Unload();
	
	// Set font size in pixels.
	void SetPixelSize( CVector2<int> size );
	void SetPixelSize( int size );
	// Set font size in points. 96 dpi is assumed.
	void SetPointSize( CVector2<float> size );
	void SetPointSize( float size );

	// Get the glyph structure for the given character code in UTF32BE encoding.
	CGlyph GetGlyph( int charCode ) const;

	// Get the glyph structure for the given character.
	// Only basic ASCII characters are supported.
	// If an non-basic ASCII character is passed, false is returned.
	bool GetGlyph( char character, CGlyph& result ) const
		{ return view.GetGlyph( character, result ); }
	// Support for wchar_t.
	// Get a glyph of a simple UTF16 symbol.
	// Is the symbol has no translation, return false.
	bool GetUnicodeGlyph( wchar_t charCode, CGlyph& result ) const
		{ return view.GetUnicodeGlyph( charCode, result ); }
	// Get a glyph of a UTF16 symbol that consists of two character codes.
	// Is the codes have no translation, return false.
	bool GetUnicodeGlyph( wchar_t hiCode, wchar_t loCode, CGlyph& result ) const
		{ return view.GetUnicodeGlyph( hiCode, loCode, result ); }

	// Initialization functions need access to the FT_Library object to fill it.
	friend class CRelibInitializer;

private:
	// Buffer with the font data.
	CArray<BYTE> fontData;
	// Font view.
	CFontEdit view;

	static FT_Library freeTypeLib;

	void cleanup();
	void detachView();

	// Copying is prohibited.
	CFontOwner( CFontOwner& ) = delete;
	void operator=( CFontOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

