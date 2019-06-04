#pragma once
#include <Redefs.h>
#include <TemplateUtils.h>
#include <Reutils.h>
#include <Map.h>

namespace Relib {

namespace RelibInternal {

extern REAPI CCriticalSection EventClassIdSection;
extern REAPI CMap<CStringView, int, CDefaultHash<CStringView>, CProcessHeap> EventClassNameToId;
//////////////////////////////////////////////////////////////////////////

// Event class instance with event id information.
template <class EventClass>
class CEventClassInstance {
public:
	static int GetEventClassId() 
		{ return classId; }

private:
	static const int classId;
	static int retrieveClassId();
};

template <class EventClass>
const int CEventClassInstance<EventClass>::classId = retrieveClassId();

template <class EventClass>
int CEventClassInstance<EventClass>::retrieveClassId()
{
	CCriticalSectionLock idLock( EventClassIdSection );
	return EventClassNameToId.GetOrCreate( typeid( EventClass ).raw_name(), EventClassNameToId.Size() ).Value();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

// Event class id utility function.
template <class EventClass>
int GetEventClassId()
{
	return RelibInternal::CEventClassInstance<EventClass>::GetEventClassId();
}

namespace RelibEvents {

// Events from the ECS.
class CEntityExpirationEvent {};

}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

