#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Owning pointer of an update or draw system. Removes the system from ECS on destruction.
template <class System>
class CSystemOwner {
public:
	CSystemOwner() = default;
	CSystemOwner( System& systemRef, CEntityComponentSystem& ecs );
	CSystemOwner( CSystemOwner<System>&& other );
	CSystemOwner& operator=( CSystemOwner<System>&& other );
	~CSystemOwner();

	System& operator*()
		{ return *systemPtr; }
	const System& operator*() const
		{ return *systemPtr; }

	System* operator->()
		{ return systemPtr; }
	const System* operator->() const
		{ return systemPtr; }

private:
	CEntityComponentSystem* ecs = nullptr;
	System* systemPtr = nullptr;

	void release();

	// Copying is prohibited.
	CSystemOwner( CSystemOwner& ) = delete;
	void operator=( CSystemOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class System>
CSystemOwner<System>::CSystemOwner( System& ref, CEntityComponentSystem& _ecs ) :
	systemPtr( &ref ),
	ecs( &_ecs )
{
}

template <class System>
CSystemOwner<System>::CSystemOwner( CSystemOwner<System>&& other ) :
	systemPtr( other.systemPtr ),
	ecs( other.ecs )
{
	other.systemPtr = nullptr;
}

template <class System>
CSystemOwner<System>::~CSystemOwner()
{
	release();
}

template <class System>
void CSystemOwner<System>::release()
{
	if( systemPtr != nullptr ) {
		ecs->removeSystem( systemPtr );
	}
}

template <class System>
CSystemOwner<System>& CSystemOwner<System>::operator=( CSystemOwner<System>&& other )
{
	release();
	systemPtr = other.systemPtr;
	ecs = other.ecs;
	other.systemPtr = nullptr;
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

