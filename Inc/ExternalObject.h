#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Interface for an object with an external name.
// Derivatives of this class can be created using CreateUniqueObject function.
class IExternalObject {
public:
	virtual ~IExternalObject() = 0;
};

inline IExternalObject::~IExternalObject()
{
}

//////////////////////////////////////////////////////////////////////////

// Base class that is externally constructible by name.
class IBaseConstructable : public IExternalObject {};

// An object that can be constructed with the specified template parameters as arguments.
// Serves as annotation for creating external objects with arguments.
template <class... Args>
class IConstructable : public IBaseConstructable {
public:
	typedef CTuple<Args...> TConstructorArgsTuple;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

