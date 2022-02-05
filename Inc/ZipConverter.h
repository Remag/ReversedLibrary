#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_ZLIB

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for inflating and deflating data. Uses ZLib for inflation/deflation algorithms.
class REAPI CZipConverter {
public:
	void ZipData( CArrayView<BYTE> data, CArray<BYTE>& result );
	void UnzipData( CArrayView<BYTE> data, CArray<BYTE>& result );

private:
	static void* allocationFunction( void* opaque, unsigned itemCoun, unsigned itemSize );
	static void freeFunction( void* opaque, void* ptr );
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif // RELIB_NO_ZLIB