#pragma once
#include <VarArgsUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A container for a number of arbitrary types.
template <class... TT>
class CTuple;

// Empty tuple specialization.
template <>
class CTuple<> {
public:
	static constexpr int Size()
		{ return 0; }
};

// Single element tuple specialization.
template <class T>
class CTuple<T> {
public:
	CTuple() = default;
	template <class Type>
	explicit CTuple( Type&& initValue ) : value( forward<Type>( initValue ) ) {}

	template <int argNum>
	using Elem = typename VarArgs::At<argNum, T>::Result;

	static constexpr int Size()
		{ return 1; }

	template <int pos>
	T& Get()
		{ static_assert( pos == 0, "Out of bounds tuple access." ); return value; }
	template <int pos>
	const T& Get() const
		{ return const_cast<CTuple<T>*>( this )->Get<pos>(); }

	template <int pos>
	void Set( T newValue )
		{ Get<pos>() = move( newValue ); }

private:
	T value;
};

//////////////////////////////////////////////////////////////////////////

// General tuple specialization.
template <class FirstType, class... TT>
class CTuple<FirstType, TT...> {
public:
	CTuple() = default;
	template <class First, class... Rest>
	CTuple( First&& first, Rest&&... rest ) : current( forward<First>( first ) ), rest( forward<Rest>( rest )... ) {}

	template <int argNum>
	using Elem = typename VarArgs::At<argNum, FirstType, TT...>::Result;

	static constexpr int Size()
		{ return sizeof...( TT ) + 1; }

	template <int pos>
	auto& Get();
	template <int pos>
	auto& Get() const
		{ return const_cast<CTuple<FirstType, TT...>*>( this )->Get<pos>(); }

	template <int pos>
	void Set( Elem<pos> newValue )
		{ Get<pos>() = move( newValue ); }

private:
	FirstType current;
	CTuple<TT...> rest;

	template <int pos>
	auto& getElem( Types::FalseType isPosCurrent );
	template <int pos>
	auto& getElem( Types::TrueType isPosCurrent );
};

//////////////////////////////////////////////////////////////////////////

template <class FirstType, class... TT>
template <int pos>
auto& CTuple<FirstType, TT...>::Get()
{
	staticAssert( pos <= sizeof...( TT ) );
	return getElem<pos>( Types::BoolType<pos == 0>() );
}

template <class FirstType, class... TT>
template <int pos>
auto& CTuple<FirstType, TT...>::getElem( Types::FalseType )
{
	return rest.Get<pos - 1>();
}

template <class FirstType, class... TT>
template <int pos>
auto& CTuple<FirstType, TT...>::getElem( Types::TrueType )
{
	return current;
}

//////////////////////////////////////////////////////////////////////////

template <class... TT>
auto CreateTuple( TT&&... types )
{
	return CTuple<Types::PureType<TT>::Result...>( forward<TT>( types )... );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

