#include <EventSystem.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <StrConversions.h>
#include <TypelessActionOwner.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CExternalEventTarget CEventSystem::AddExternalEventTarget( int classId, const IExternalObject* eventAction )
{
	if( classId >= listeners.Size() ) {
		listeners.IncreaseSize( classId + 1 );
	}
	listeners[classId].Add( eventAction );

	return CExternalEventTarget( *this, eventAction, classId );
}

void CEventSystem::removeListener( int classId, const IExternalObject* listenerObject )
{
	auto& listenerList = listeners[classId];
	const int listenerCount = listenerList.Size();
	for( int i = 0; i < listenerCount; i++ ) {
		if( listenerList[i] == listenerObject ) {
			listenerList.DeleteAt( i );
			return;
		}
	}

	assert( false );
}

void CEventSystem::removeListener( int classId, const CTypelessActionOwner& listenerObject )
{
	removeListener( classId, listenerObject.GetActionObject() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
