#pragma once
#include <Invoke.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

template <int... seq>
struct CIntSeq {};

template <int currentPos, int size, int... seq>
struct CIntSeqGeneratorImpl {
	typedef typename CIntSeqGeneratorImpl<currentPos + 1, size, seq..., currentPos>::ResultSeq ResultSeq;
};

template <int size, int... seq>
struct CIntSeqGeneratorImpl<size, size, seq...> {
	typedef CIntSeq<seq...> ResultSeq;
};

template <int size>
struct CIntSeqGenerator {
	typedef typename CIntSeqGeneratorImpl<0, size>::ResultSeq ResultSeq;
};

template <class Callable, class... TupleArgs, int... intSeq>
auto invokeAction( Callable& action, const CTuple<TupleArgs...>& args, CIntSeq<intSeq...> )
{ 
	return Invoke( action, args.Get<intSeq>()... );
}

template <class Callable, class... TupleArgs, int... intSeq>
auto invokeAction( Callable& action, CTuple<TupleArgs...>&& args, CIntSeq<intSeq...> )
{ 
	return Invoke( action, move( args.Get<intSeq>() )... );
}

template <class Callable, class... TupleArgs, int... intSeq>
auto invokeAction( Callable& action, CTuple<TupleArgs...>& args, CIntSeq<intSeq...> )
{ 
	return Invoke( action, args.Get<intSeq>()... );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Call a given action with a tuple for its arguments.
template <class Callable, class... TupleArgs>
auto TupleInvoke( Callable& action, CTuple<TupleArgs...>& args )
{
	return RelibInternal::invokeAction( action, args, typename RelibInternal::CIntSeqGenerator<sizeof...( TupleArgs )>::ResultSeq() );
}

template <class Callable, class... TupleArgs>
auto TupleInvoke( Callable& action, CTuple<TupleArgs...>&& args )
{
	return RelibInternal::invokeAction( action, move( args ), typename RelibInternal::CIntSeqGenerator<sizeof...( TupleArgs )>::ResultSeq() );
}

template <class Callable, class... TupleArgs>
auto TupleInvoke( Callable& action, const CTuple<TupleArgs...>& args )
{
	return RelibInternal::invokeAction( action, args, typename RelibInternal::CIntSeqGenerator<sizeof...( TupleArgs )>::ResultSeq() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

