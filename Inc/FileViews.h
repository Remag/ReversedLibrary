#pragma once
#include <FileOperations.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A non-owning file read class.
class CFileReadView : public RelibInternal::CFileReadOperations {
public:
	using CFileReadOperations::CFileReadOperations;
};

// A non-owning file write class.
class CFileWriteView : public RelibInternal::CFileWriteOperations {
public:
	using CFileWriteOperations::CFileWriteOperations;
};

// A non-owning file read-write class.
class CFileReadWriteView : public RelibInternal::CFileReadWriteOperations {
public:
	using CFileReadWriteOperations::CFileReadWriteOperations;

	operator CFileReadView() const
		{ return CFileReadView( getFileHandle() ); }
	operator CFileWriteView() const
		{ return CFileReadWriteView( getFileHandle() ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

