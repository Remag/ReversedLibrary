#pragma once
#include <ComponentUtils.h>

namespace Relib {

namespace RelibInternal {
	template <class ComponentClass>
	class CComponentClassInstance;
}

//////////////////////////////////////////////////////////////////////////

// Base component class. All component classes should inherit from this class.
// Entities of this class can hold an arbitrary subset of components with no penalty.
class CBaseComponentClass : public IComponentClass {
public:
	CBaseComponentClass() = default;

	int GetUniqueId() const
		{ return uniqueId; }

	virtual int GetClassId() const override final
		{ return uniqueId; }

	template <class ComponentClass>
	friend class RelibInternal::CComponentClassInstance;

private:
	int uniqueId;

	void initComponentClass( int _uniqueId )
		{ uniqueId = _uniqueId; }

	// Copying is prohibited.
	CBaseComponentClass( CBaseComponentClass& ) = delete;
	void operator=( CBaseComponentClass& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

