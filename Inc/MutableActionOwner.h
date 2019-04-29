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
class CMutableActionOwner;

template <class ReturnType, class... Args>
class CMutableActionOwner<ReturnType( Args... )> {
public:
	CMutableActionOwner() = default;
	template <class Callable>
	CMutableActionOwner( Callable func );
	explicit CMutableActionOwner( CTypelessActionOwner typelessAction );
	CMutableActionOwner( CMutableActionOwner&& other );
	CMutableActionOwner& operator=( CMutableActionOwner&& other );
	~CMutableActionOwner();

	bool IsNull() const
		{ return callable == nullptr; }

	IMutableAction<ReturnType( Args... )>* GetAction()
		{ return callable; }
	const IMutableAction<ReturnType( Args... )>* GetAction() const
		{ return callable; }

	ReturnType Invoke( Args... args ) 
		{ assert( !IsNull() ); return callable->Invoke( forward<Args>( args )... ); }
	ReturnType operator()( Args... args )
		{ return Invoke( forward<Args>( args )... ); }

	// Typeless owners need to be able to typify themselves.
	friend class CTypelessActionOwner;
	
private:
	IMutableAction<ReturnType( Args... )>* callable = nullptr;
	int callableSize = 0;

	template <class C>
	void initCallableAsAction( C func, Types::TrueType );
	template <class C>
	void initCallableAsAction( C&&, Types::FalseType ) {}

	template <class C>
	void initCallableAsClass( C func, Types::TrueType );
	template <class C>
	void initCallableAsClass( C&&, Types::FalseType ) {}

	CMutableActionOwner( IExternalObject* typelessAction, int size );

	// Copying is prohibited.
	CMutableActionOwner( CMutableActionOwner& )  = delete;
	void operator=( CMutableActionOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ReturnType, class... Args>
template <class Callable>
CMutableActionOwner<ReturnType( Args... )>::CMutableActionOwner( Callable func )
{
	typedef Types::IsDerivedFrom<Callable, IMutableAction<ReturnType( Args... )>> TIsAction;
	typedef Types::BoolType<!Types::IsDerivedFrom<Callable, IMutableAction<ReturnType( Args... )>>::Result> TIsNotAction;
	initCallableAsAction( move( func ), TIsAction() );
	initCallableAsClass( move( func ), TIsNotAction() );
	assert( callable != nullptr && callableSize > 0 );
}

template <class ReturnType, class... Args>
template <class C>
void CMutableActionOwner<ReturnType( Args... )>::initCallableAsAction( C func, Types::TrueType )
{
	assert( callable == nullptr && callableSize == 0 );
	callableSize = sizeof( C );
	void* callablePtr = RelibInternal::GetActionOwnerAllocator().Allocate( callableSize );
	callable = ::new( callablePtr ) C( move( func ) );
}

template <class ReturnType, class... Args>
template <class C>
void CMutableActionOwner<ReturnType( Args... )>::initCallableAsClass( C c, Types::TrueType )
{
	// Initialize the action with a non-action callable.
	// Some callables may have a templated operator().
	// For these callables CAction cannot determine the callable parameters.
	// Therefore CBaseAction is used.
	assert( callable == nullptr && callableSize == 0 );
	typedef RelibInternal::CBaseMutableAction<C, ReturnType, CTuple<Args...>> TBaseAction;
	callableSize = sizeof( TBaseAction );
	void* callablePtr = RelibInternal::GetActionOwnerAllocator().Allocate( callableSize );
	callable = ::new( callablePtr ) TBaseAction( move( c ) );
}

template <class ReturnType, class... Args>
CMutableActionOwner<ReturnType( Args... )>::CMutableActionOwner( CTypelessActionOwner typelessAction ) :
	callable( static_cast<IMutableAction<ReturnType( Args... )>*>( typelessAction.action ) ),
	callableSize( typelessAction.actionSize )
{
	assert( dynamic_cast<IMutableAction<ReturnType( Args... )>*>( typelessAction.action ) == callable );
	typelessAction.action = nullptr;
	typelessAction.actionSize = 0;
}

template <class ReturnType, class... Args>
CMutableActionOwner<ReturnType( Args... )>& CMutableActionOwner<ReturnType( Args... )>::operator=( CMutableActionOwner<ReturnType( Args... )>&& other )
{
	swap( callable, other.callable );
	swap( callableSize, other.callableSize );
	return *this;
}

template <class ReturnType, class... Args>
CMutableActionOwner<ReturnType( Args... )>::CMutableActionOwner( CMutableActionOwner&& other ) :
	callable( other.callable ),
	callableSize( other.callableSize )
{
	other.callable = nullptr;
	other.callableSize = 0;
}

template <class ReturnType, class... Args>
CMutableActionOwner<ReturnType( Args... )>::~CMutableActionOwner()
{
	if( !IsNull() ) {
		callable->~IMutableAction();
		RelibInternal::GetActionOwnerAllocator().Free( callable, callableSize );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

