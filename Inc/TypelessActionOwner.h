#pragma once
#include <Redefs.h>
#include <GeneralBlockAllocator.h>
#include <TemplateUtils.h>
#include <Remath.h>

namespace Relib {

namespace RelibInternal {

// Making this variable a part of the class will crash the compiler.
static const int smallActionOwnerBlockSize = Relib::CeilTo( 2 * sizeof( void* ) + sizeof( int ), 8 );
class REAPI CActionOwnerAllocator {
public:
	CActionOwnerAllocator() = default;
	
	void* Allocate( int size );
	void Free( void* ptr, int allocSize );

private:
	CCriticalSection allocationLock;
	// Manager for regular actions.
	CGeneralBlockAllocator<smallActionOwnerBlockSize, AllocatorAlignment, CProcessHeap, CDynamicBlockResizeStrategy<smallActionOwnerBlockSize, 16>> smallActionManager;
	// Manager for bigger actions like the results of a Bind function.
	static const int biggerBlockSize = 48;
	CGeneralBlockAllocator<biggerBlockSize, AllocatorAlignment, CProcessHeap, CDynamicBlockResizeStrategy<biggerBlockSize, 16>> biggerActionManager;
};

REAPI CActionOwnerAllocator& GetActionOwnerAllocator();

}

//////////////////////////////////////////////////////////////////////////

// Action container. Stores and deletes a given action using CActionOwnerAllocator.
class CTypelessActionOwner {
public:
	CTypelessActionOwner() = default;
	template <class ActionType>
	CTypelessActionOwner( CActionOwner<ActionType> actionOwner );
	template <class ActionType>
	CTypelessActionOwner( CMutableActionOwner<ActionType> actionOwner );
	CTypelessActionOwner( CTypelessActionOwner&& other );
	CTypelessActionOwner& operator=( CTypelessActionOwner other );
	~CTypelessActionOwner();

	const IExternalObject* GetActionObject() const
		{ return action; }

	// Action classes can untypify themselves.
	template <class ActionType>
	friend class CActionOwner;
	template <class ActionType>
	friend class CMutableActionOwner;

private:
	IExternalObject* action = nullptr;
	int actionSize = 0;

	// Copying is prohibited.
	CTypelessActionOwner( CTypelessActionOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ActionType>
CTypelessActionOwner::CTypelessActionOwner( CActionOwner<ActionType> actionOwner ) :
	action( actionOwner.callable ),
	actionSize( actionOwner.callableSize )
{
	actionOwner.callable = nullptr;
	actionOwner.callableSize = 0;
}

template <class ActionType>
CTypelessActionOwner::CTypelessActionOwner( CMutableActionOwner<ActionType> actionOwner ) :
	action( actionOwner.callable ),
	actionSize( actionOwner.callableSize )
{
	actionOwner.callable = nullptr;
	actionOwner.callableSize = 0;
}

inline CTypelessActionOwner::~CTypelessActionOwner()
{
	if( action != nullptr ) {
		action->~IExternalObject();
		RelibInternal::GetActionOwnerAllocator().Free( action, actionSize );
	}
}

inline CTypelessActionOwner::CTypelessActionOwner( CTypelessActionOwner&& other ) :
	action( other.action ),
	actionSize( other.actionSize )
{
	other.action = nullptr;
	other.actionSize = 0;
}

inline CTypelessActionOwner& CTypelessActionOwner::operator=( CTypelessActionOwner other )
{
	swap( action, other.action );
	swap( actionSize, other.actionSize );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

