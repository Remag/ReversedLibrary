#pragma once
#include <Redefs.h>
#include <Action.h>
#include <ActionImpl.h>
#include <TypelessActionOwner.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Action that owns an arbitrary action.
// Data is allocated using the block manager that is large enough to contain the biggest library action implementation.
template <class Callable>
class CActionOwner;

template <class ReturnType, class... Args>
class CActionOwner<ReturnType( Args... )> {
public:
	CActionOwner() = default;
	template <class Callable>
	CActionOwner( Callable func );
	explicit CActionOwner( CTypelessActionOwner typelessAction );
	CActionOwner( CActionOwner&& other );
	CActionOwner& operator=( CActionOwner&& other );
	~CActionOwner();

	bool IsNull() const
		{ return callable == nullptr; }

	IAction<ReturnType( Args... )>* GetAction()
		{ return callable; }
	const IAction<ReturnType( Args... )>* GetAction() const
		{ return callable; }

	ReturnType Invoke( Args... args ) const 
		{ assert( !IsNull() ); return callable->Invoke( forward<Args>( args )... ); }
	ReturnType operator()( Args... args ) const
		{ return Invoke( forward<Args>( args )... ); }

	// Typeless owners need to be able to typify themselves.
	friend class CTypelessActionOwner;
	
private:
	IAction<ReturnType( Args... )>* callable = nullptr;
	int callableSize = 0;

	template <class C>
	void initCallableAsAction( C func, Types::TrueType );
	template <class C>
	void initCallableAsAction( C&&, Types::FalseType ) {}

	template <class C>
	void initCallableAsClass( C func, Types::TrueType );
	template <class C>
	void initCallableAsClass( C&&, Types::FalseType ) {}

	// Copying is prohibited.
	CActionOwner( CActionOwner& )  = delete;
	void operator=( CActionOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ReturnType, class... Args>
template <class Callable>
CActionOwner<ReturnType( Args... )>::CActionOwner( Callable func )
{
	typedef Types::IsDerivedFrom<Callable, IAction<ReturnType( Args... )>> TIsAction;
	typedef Types::BoolType<!Types::IsDerivedFrom<Callable, IAction<ReturnType( Args... )>>::Result> TIsNotAction;
	initCallableAsAction( move( func ), TIsAction() );
	initCallableAsClass( move( func ), TIsNotAction() );
	assert( callable != nullptr && callableSize > 0 );
}

template <class ReturnType, class... Args>
template <class C>
void CActionOwner<ReturnType( Args... )>::initCallableAsAction( C func, Types::TrueType )
{
	assert( callable == nullptr && callableSize == 0 );
	callableSize = sizeof( C );
	void* callablePtr = RelibInternal::GetActionOwnerAllocator().Allocate( callableSize );
	callable = ::new( callablePtr ) C( move( func ) );
}

template <class ReturnType, class... Args>
template <class C>
void CActionOwner<ReturnType( Args... )>::initCallableAsClass( C c, Types::TrueType )
{
	// Initialize the action with a non-action callable.
	// Some callables may have a templated operator().
	// For these callables CAction cannot determine the callable parameters.
	// Therefore CBaseAction is used.
	assert( callable == nullptr && callableSize == 0 );
	typedef RelibInternal::CBaseAction<C, ReturnType, CTuple<Args...>> TBaseAction;
	callableSize = sizeof( TBaseAction );
	void* callablePtr = RelibInternal::GetActionOwnerAllocator().Allocate( callableSize );
	callable = ::new( callablePtr ) TBaseAction( move( c ) );
}

template <class ReturnType, class... Args>
CActionOwner<ReturnType( Args... )>::CActionOwner( CTypelessActionOwner typelessAction ) :
	callable( static_cast<IAction<ReturnType( Args... )>*>( typelessAction.action ) ),
	callableSize( typelessAction.actionSize )
{
	assert( dynamic_cast<IAction<ReturnType( Args... )>*>( typelessAction.action ) == callable );
	typelessAction.action = nullptr;
	typelessAction.actionSize = 0;
}

template <class ReturnType, class... Args>
CActionOwner<ReturnType( Args... )>& CActionOwner<ReturnType( Args... )>::operator=( CActionOwner<ReturnType( Args... )>&& other )
{
	swap( callable, other.callable );
	swap( callableSize, other.callableSize );
	return *this;
}

template <class ReturnType, class... Args>
CActionOwner<ReturnType( Args... )>::CActionOwner( CActionOwner&& other ) :
	callable( other.callable ),
	callableSize( other.callableSize )
{
	other.callable = nullptr;
	other.callableSize = 0;
}

template <class ReturnType, class... Args>
CActionOwner<ReturnType( Args... )>::~CActionOwner()
{
	if( !IsNull() ) {
		callable->~IAction();
		RelibInternal::GetActionOwnerAllocator().Free( callable, callableSize );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

