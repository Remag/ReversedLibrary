#pragma once
#include <Future.h>
#include <Ptr.h>
#include <StaticAllocators.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class representing a value in the process of being created.
template <class T> 
class CPromise {
public:
	CPromise();
	CPromise( CPromise<T>&& other ) = default;
	~CPromise();

	bool IsConnectedWith( const CFuture<T>& future ) const;
	CFuture<T> GetFuture() const;

	template <class... Args> 
	void CreateValue( Args&&... args );

private:
	CSharedPtr<RelibInternal::CFutureSharedState<T>, CProcessHeap> sharedState;

	// Copying is prohibited.
	CPromise( const CPromise& ) = delete;
	void operator=( const CPromise& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
inline CPromise<T>::CPromise() :
	sharedState( CreateShared<RelibInternal::CFutureSharedState<T>, CProcessHeap>() )
{
}

template<class T>
inline CPromise<T>::~CPromise()
{
	if( sharedState != nullptr ) {
		sharedState->Abandon();
	}
}

template<class T>
inline bool CPromise<T>::IsConnectedWith( const CFuture<T>& future ) const
{
	return future.sharedState == sharedState;
}

template<class T>
inline CFuture<T> CPromise<T>::GetFuture() const
{
	return CFuture<T>( sharedState );
}

template<class T>
template<class ...Args>
inline void CPromise<T>::CreateValue( Args&& ...args )
{
	sharedState->CreateValue( forward<Args>( args )... );
	sharedState = nullptr;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.