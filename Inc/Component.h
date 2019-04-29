#pragma once
#include <Redefs.h>
#include <Atomic.h>
#include <TemplateUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// An entity data member. Objects of this class serve as an identifier of entities' data at a particular position in a group.
class CBaseComponent {
public:
	typedef void ( *TConstructComponentData )( void* dataPtr, int dataByteCount );
	typedef void ( *TDestroyComponentData )( void* dataPtr, int dataByteCount );
	typedef void ( *TMoveConstructComponentData )( void* srcPtr, void* destPtr, int dataByteCount );
	typedef void ( *TMoveAssignComponentData )( void* srcPtr, void* destPtr );

	explicit CBaseComponent( int _size, TConstructComponentData _constructPtr, TDestroyComponentData _destroyPtr, TMoveConstructComponentData _movePtr, TMoveAssignComponentData _assignPtr ) :
		uniqueId( createUniqueId() ), size( _size ), constructPtr( _constructPtr ), destroyPtr( _destroyPtr ), moveConstructPtr( _movePtr ), moveAssignPtr( _assignPtr ) {}

	// Get component data element size.
	int GetSize() const 
		{ return size; }

	// A unique component identifier.
	int GetComponentId() const
		{ return uniqueId; }

	// Component data construction, destruction and movement functions. May be null.
	TConstructComponentData GetConstructFunction() const
		{ return constructPtr; }
	TDestroyComponentData GetDestroyFunction() const
		{ return destroyPtr; }	
	TMoveConstructComponentData GetMoveConstructFunction() const
		{ return moveConstructPtr; }	
	TMoveAssignComponentData GetMoveAssignFunction() const
		{ return moveAssignPtr; }

private:
	TConstructComponentData constructPtr;
	TDestroyComponentData destroyPtr;
	TMoveConstructComponentData moveConstructPtr;
	TMoveAssignComponentData moveAssignPtr;
	int uniqueId;
	int size;

	int createUniqueId() const;
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {
	extern REAPI CAtomic<int> CurrentComponentId;
}	// namespace RelibInternal.

inline int CBaseComponent::createUniqueId() const
{
	return RelibInternal::CurrentComponentId.PostIncrement();
}

//////////////////////////////////////////////////////////////////////////

template <class T>
class CComponent : public CBaseComponent {
public:
	CComponent() : CBaseComponent( sizeof( T ), createConstructFunction(), createDestroyFunction(), createMoveConstructFunction(), createMoveAssignFunction() ) {}

	static bool IsTNonTrivial();

private:
	static TDestroyComponentData createConstructFunction();
	static TDestroyComponentData createDestroyFunction();
	static TMoveConstructComponentData createMoveConstructFunction();
	static TMoveAssignComponentData createMoveAssignFunction();

	static bool isTNonTrivialOrEnum( Types::TrueType enumMark );
	static bool isTNonTrivialOrEnum( Types::FalseType enumMark );

	static void constructDataComponent( void* data, int dataByteCount );
	static void destroyDataComponent( void* data, int dataByteCount );
	static void moveConstructDataComponent( void* src, void* dest, int dataByteCount );
	static void moveAssignDataComponent( void* src, void* dest );

	// Copying is prohibited.
	CComponent( CComponent& ) = delete;
	void operator=( CComponent& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
bool CComponent<T>::IsTNonTrivial()
{
	// Enums are checked separately because HasTrivialDestructor does not work for undefined enums.
	return isTNonTrivialOrEnum( Types::IsEnum<T>() );
}

template <class T>
bool CComponent<T>::isTNonTrivialOrEnum( Types::FalseType )
{
	return !Types::HasTrivialDestructor<T>::Result && !Types::HasTrivialMoveConstructor<T>::Result;
}

template <class T>
bool CComponent<T>::isTNonTrivialOrEnum( Types::TrueType )
{
	return false;
}

template <class T>
CBaseComponent::TConstructComponentData CComponent<T>::createConstructFunction()
{
	return IsTNonTrivial() ? constructDataComponent : nullptr;
}

template <class T>
void CComponent<T>::constructDataComponent( void* data, int dataByteCount )
{
	BYTE* byteData = static_cast<BYTE*>( data );
	for( int pos = 0; pos < dataByteCount; pos += sizeof( T ) ) {
		::new( byteData + pos ) T();
	}
}

template <class T>
CBaseComponent::TDestroyComponentData CComponent<T>::createDestroyFunction()
{
	return IsTNonTrivial() ? destroyDataComponent : nullptr;
}

template <class T>
void CComponent<T>::destroyDataComponent( void* data, int dataByteCount )
{
	BYTE* byteData = static_cast<BYTE*>( data );
	for( int pos = 0; pos < dataByteCount; pos += sizeof( T ) ) {
		reinterpret_cast<T*>( byteData + pos )->~T();
	}
}

template <class T>
CBaseComponent::TMoveConstructComponentData CComponent<T>::createMoveConstructFunction()
{
	return IsTNonTrivial() ? moveConstructDataComponent : nullptr;
}

template <class T>
void CComponent<T>::moveConstructDataComponent( void* src, void* dest, int dataByteCount )
{
	BYTE* byteSrc = static_cast<BYTE*>( src );
	BYTE* byteDest = static_cast<BYTE*>( dest );
	for( int pos = 0; pos < dataByteCount; pos += sizeof( T ) ) {
		const auto castSrc = reinterpret_cast<T*>( byteSrc + pos );
		::new( byteDest + pos ) T( move( *castSrc ) );
	}
}

template <class T>
CBaseComponent::TMoveAssignComponentData CComponent<T>::createMoveAssignFunction()
{
	return IsTNonTrivial() ? moveAssignDataComponent : nullptr;
}

template <class T>
void CComponent<T>::moveAssignDataComponent( void* src, void* dest )
{
	*static_cast<T*>( dest ) = move( *static_cast<T*>( src ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

