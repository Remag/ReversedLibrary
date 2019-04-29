#include <Entity.h>
#include <EntityRef.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CEntityConstRef CConstEntity::CreateReference() const
{
	return CEntityConstRef( fullData, fullData->Generation );
}

CEntityRef CEntity::CreateReference()
{
	return CEntityRef( fullData, fullData->Generation );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.



