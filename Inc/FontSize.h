#pragma once
#include <Redefs.h>

namespace Relib {

struct FT_Size_Rec_;
typedef struct FT_SizeRec_* FT_Size;

//////////////////////////////////////////////////////////////////////////

class REAPI CFontSizeView {
public:
	explicit CFontSizeView( FT_Size handle ) : sizeHandle( handle ) {}

	FT_Size GetHandle() const
		{ return sizeHandle; }

private:
	FT_Size sizeHandle;
};

//////////////////////////////////////////////////////////////////////////

// Font size object. Can be created and switched in the font class.
class REAPI CFontSizeOwner {
public:
	CFontSizeOwner() = default;
	CFontSizeOwner( CFontSizeOwner&& other );
	CFontSizeOwner& operator=( CFontSizeOwner&& other );
	~CFontSizeOwner();
	
	operator CFontSizeView() const
		{ return CFontSizeView( sizeHandle ); }
	FT_Size GetHandle() const
		{ return sizeHandle; }

	// Only font objects can create size objects.
	friend class CFontView;

private:
	FT_Size sizeHandle = nullptr;

	explicit CFontSizeOwner( FT_Size handle ) : sizeHandle( handle ) {}
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

