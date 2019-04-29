#pragma once
#include <Redefs.h>
#include <ExternalObject.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Interface for objects that support loading and storing to archive.
class ISerializable : public IExternalObject {
public:
	// Serialize the inner data of the class.
	virtual void Serialize( CArchiveReader& archive ) = 0;
	virtual void Serialize( CArchiveWriter& archive ) const = 0;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

