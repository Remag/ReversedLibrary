#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for inflating and deflating data. Uses ZLib for inflation/deflation algorithms.
class REAPI CZipConverter {
public:
	void ZipData( CArrayView<BYTE> data, CArray<BYTE>& result );
	void UnzipData( CArrayView<BYTE> data, CArray<BYTE>& result );

private:
	static void* allocationFunction( void* opaque, size_t itemCoun, size_t itemSize );
	static void freeFunction( void* opaque, void* ptr );
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

