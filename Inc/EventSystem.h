#pragma once
#include <Array.h>
#include <ArrayBuffer.h>
#include <EventUtils.h>
#include <ActionOwner.h>

namespace Relib {

class CEventSystem;
//////////////////////////////////////////////////////////////////////////

// Generic event.
class IEvent {
public:
	virtual ~IEvent() {}

	virtual int GetClassId() const = 0;
	virtual void DispatchListenerActions( CArrayView<const IExternalObject*> actions ) const = 0;
};

//////////////////////////////////////////////////////////////////////////

// An event of the given class.
template <class EventClass>
class CEvent : public IEvent {
public:
	typedef EventClass TEventClass;

	CEvent() = default;
	CEvent( CEvent<EventClass>&& ) = default;
	CEvent<EventClass>& operator=( CEvent<EventClass>&& ) = default;

	// An event class. Only listeners of this class will be notified of the event.
	static int GetEventClassId()
		{ return RelibInternal::CEventClassInstance<EventClass>::GetEventClassId(); }

	virtual int GetClassId() const override final
		{ return GetEventClassId(); }
	virtual void DispatchListenerActions( CArrayView<const IExternalObject *> actions ) const override final;
};

//////////////////////////////////////////////////////////////////////////

template<class EventClass>
void CEvent<EventClass>::DispatchListenerActions( CArrayView<const IExternalObject*> actions ) const
{
	for( auto actionPtr : actions ) {
		const auto eventAction = static_cast<const IAction<void( const EventClass& )>*>( actionPtr );
		const auto& realEvent = static_cast<const EventClass&>( *this );
		eventAction->Invoke( realEvent );
	}
}

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// An event target owner. Unregisters an event target from the event system on destruction.
template <class ActionType>
class CBaseEventTarget {
public:
	CBaseEventTarget() = default;
	CBaseEventTarget( CEventSystem& events, ActionType targetAction, int classId ) : eventSystem( &events ), actionObject( move( targetAction ) ), eventClassId( classId ) {}
	CBaseEventTarget( CBaseEventTarget&& other ) : eventSystem( other.eventSystem ), actionObject( move( other.actionObject ) ), eventClassId( other.eventClassId ) 
		{ other.eventClassId = NotFound; }
	CBaseEventTarget& operator=( CBaseEventTarget other );
	~CBaseEventTarget();

	const ActionType& GetAction() const
		{ return actionObject; }

private:
	CEventSystem* eventSystem = nullptr;
	ActionType actionObject{};
	int eventClassId = NotFound;

	// Copying is prohibited.
	CBaseEventTarget( CBaseEventTarget& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ActionType>
CBaseEventTarget<ActionType>::~CBaseEventTarget()
{
	if( eventClassId != NotFound ) {
		eventSystem->removeListener( eventClassId, actionObject );
	}
}

template <class ActionType>
CBaseEventTarget<ActionType>& CBaseEventTarget<ActionType>::operator=( CBaseEventTarget<ActionType> other )
{
	swap( eventSystem, other.eventSystem );
	swap( actionObject, other.actionObject );
	swap( eventClassId, other.eventClassId );
	return *this;
}

}	// namespace RelibInternal.

typedef RelibInternal::CBaseEventTarget<CTypelessActionOwner> CEventTarget;
typedef RelibInternal::CBaseEventTarget<const IExternalObject*> CExternalEventTarget;

//////////////////////////////////////////////////////////////////////////

// An event system. Registers listeners and sends notifications.
// All events are separated into classes. Listeners can register for an event of a particular class.
// This class does not have internal multithreading support.
class REAPI CEventSystem {
public:
	CEventSystem() = default; 

	template <class EventClass>
	int GetEventClassId() const;

	template <class EventClass>
	bool HasListeners() const;
	
	template <class ActionType>
	CEventTarget AddEventTarget( ActionType&& eventAction );
	CExternalEventTarget AddExternalEventTarget( int eventClassId, const IExternalObject* eventAction );

	// Notify all the listeners.
	template <class Event>
	void Notify( const Event& e );
	// Notify all the listeners using the generic event.
	void NotifyDynamic( const IEvent& e );

	// Event target needs access to remove the action target from the list of listeners.
	template <class ActionType>
	friend class RelibInternal::CBaseEventTarget;

private:
	// All the listeners divided by class.
	CArray<CArray<const IExternalObject*>> listeners;

	void removeListener( int classId, const IExternalObject* listenerObject );
	void removeListener( int classId, const CTypelessActionOwner& listenerObject );

	// Copying is prohibited.
	CEventSystem( CEventSystem& ) = delete;
	void operator=( CEventSystem& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class EventClass>
int CEventSystem::GetEventClassId() const
{
	return RelibInternal::CEventClassInstance<EventClass>::GetEventClassId();
}

template <class EventClass>
bool CEventSystem::HasListeners() const
{
	const auto classId = GetEventClassId<EventClass>();
	return classId < listeners.Size() && !listeners[classId].IsEmpty();
}

template <class ActionType>
CEventTarget CEventSystem::AddEventTarget( ActionType&& eventAction )
{
	staticAssert( ( Types::IsSame<Types::FunctionInfo<ActionType>::ReturnType, void>::Result ) );
	staticAssert( Types::FunctionInfo<ActionType>::ArgCount == 1 );
	using TEventArgRef = typename Types::FunctionInfo<ActionType>::template ArgTypeAt<0>;
	using TEventArg = typename Types::PureType<TEventArgRef>::Result;
	typedef typename TEventArg::TEventClass TArgEventClass;
	const int classId = GetEventClassId<TArgEventClass>();
	CActionOwner<void( const TEventArg& )> actionOwner( forward<ActionType>( eventAction ) );
	CTypelessActionOwner typelessAction( move( actionOwner ) );

	assert( classId >= 0 );

	if( classId >= listeners.Size() ) {
		listeners.IncreaseSize( classId + 1 );
	}
	listeners[classId].Add( typelessAction.GetActionObject() );

	return CEventTarget( *this, move( typelessAction ), classId );
}

template <class Event>
void CEventSystem::Notify( const Event& e )
{
	typedef typename Event::TEventClass TBaseEventClass;
	staticAssert( ( Types::IsDerivedFrom<Event, CEvent<TBaseEventClass>>::Result ) );
	const int classId = e.GetEventClassId();
	if( classId >= listeners.Size() ) {
		return;
	}
	auto& listenerList = listeners[classId];
	for( int i = 0; i < listenerList.Size(); i++ ) {
		auto listener = listenerList[i];
		const auto eventAction = static_cast<const IAction<void( const Event& )>*>( listener );
		eventAction->Invoke( e );
	}
}

inline void CEventSystem::NotifyDynamic( const IEvent& e )
{
	const int classId = e.GetClassId();
	if( classId >= listeners.Size() ) {
		return;
	}
	e.DispatchListenerActions( listeners[classId] );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

