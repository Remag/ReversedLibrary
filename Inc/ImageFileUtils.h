#pragma once
#include <Redefs.h>
#include <Color.h>
#include <Array.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

struct CStaticImageData {
	CArray<CColor> Colors;
	CVector2<int> ImageSize;
};

//////////////////////////////////////////////////////////////////////////

struct CImageFrameData {
	CArray<CColor> Colors;
	int FrameEndTimeMs;
};

//////////////////////////////////////////////////////////////////////////

struct CAnimatedImageData {
	CArray<CImageFrameData> Frames;
	CVector2<int> ImageSize;
};

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.
