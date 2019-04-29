#pragma once
#include <TemplateUtils.h>

namespace Relib {

// Variadic template utilities.
namespace VarArgs {

//////////////////////////////////////////////////////////////////////////

// First element type.
template <class Arg0, class... Args>
struct First {
	typedef Arg0 Result;
};

// Last element type.
template <class... Args>
struct Last;

template <class ArgLast>
struct Last<ArgLast> {
	typedef ArgLast Result;
};

template <class Arg0, class... Args>
struct Last<Arg0, Args...> {
	typename typedef Last<Args...>::Result Result;
};

//////////////////////////////////////////////////////////////////////////

// Nth element Type.
template <int N, class... Args>
struct At;

template <class Arg0, class... Args>
struct At<0, Arg0, Args...> {
	typedef Arg0 Result;
};

template <int N, class Arg0, class... Args>
struct At<N, Arg0, Args...> {
	typename typedef At<N - 1, Args...>::Result Result;
};

//////////////////////////////////////////////////////////////////////////

// Position of the given type among the given arguments. One of the types has to be the same.
namespace RelibInternal {

template <int, class, class...>
struct BaseTargetPos;

template <int currentPos, class Target, class Arg0, class... Args>
struct BaseTargetPos<currentPos, Target, Arg0, Args...> {
	static const int Result = Types::IsSame<Target, Arg0>::Result ? currentPos : BaseTargetPos<currentPos + 1, Target, Args...>::Result;
};

template <int currentPos, class Target>
struct BaseTargetPos<currentPos, Target> {
	static const int Result = NotFound;
};

}

template <class Target, class... Args>
struct TargetPos {
	static const int Result = RelibInternal::BaseTargetPos<0, Target, Args...>::Result;
	static_assert( Result != NotFound, "Target class not found." );
};

//////////////////////////////////////////////////////////////////////////

// Total size of all arguments. Alignment is not considered.
template <class... Args>
struct SizeOfAll;

template <class Arg0>
struct SizeOfAll<Arg0> {
	static const int Result = sizeof( Arg0 );
};

template <class Arg0, class... Args>
struct SizeOfAll<Arg0, Args...> {
	static const int Result = sizeof( Arg0 ) + SizeOfAll<Args...>::Result;
};

}

}	// namespace Relib.