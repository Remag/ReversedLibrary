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

	// Create an object with size information.
	CFontSizeOwner CreateSizeObject( float pxSize ) const;
	CFontSizeOwner CreateSizeObject( CVector2<float> pxSize ) const;

	// Get the glyph structure for the given character code in UTF32BE encoding.
	CGlyph GetGlyph( int charCode, CFontSizeView fontSize ) const;

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

	// Create an object with size information.
	CFontSizeOwner CreateSizeObject( float pxSize ) const;
	CFontSizeOwner CreateSizeObject( CVector2<float> pxSize ) const;
	// Get the glyph structure for the given character code in UTF32BE encoding.
	CGlyph GetGlyph( int charCode, CFontSizeView fontSize ) const;

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

