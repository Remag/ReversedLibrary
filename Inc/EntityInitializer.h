#pragma once
#include <MemoryOwner.h>
#include <ComponentGroup.h>
#include <EntityRef.h>

namespace Relib {

class CEntityRef;
struct CFullEntityData;
//////////////////////////////////////////////////////////////////////////

// Memory structures required to initialize an entity.
// Created by the user and passed to the ECS to reduce allocation count.
class REAPI CEntityInitializationData {
public:
	CEntityInitializationData() = default;
	CEntityInitializationData( CEntityInitializationData&& ) = default;
	CEntityInitializationData& operator=( CEntityInitializationData&& ) = default;
	~CEntityInitializationData()
		{ callDestructors(); }

	// Only entity initializers can access the data.
	friend class CEntityInitializer;
	friend class RelibInternal::CFilledEntityData;

private:
	struct CTrivialUtilityData {
		int Offset;
		int CompId;
	};
	struct CDestructibleUtilityData {
		CBaseComponent::TDestroyComponentData DestroyPtr;
		CBaseComponent::TMoveConstructComponentData MovePtr;
		int Offset;
		int CompId;
	};

	// Components present in the initialization data.
	CComponentGroup components;
	// The data is separated into two segments.
	// Trivial data collects components with a trivial destructor.
	// Destructible data collects other components.
	CArray<CTrivialUtilityData> trivialUtilityData;
	CMemoryOwner<> trivialData;
	int trivialDataOffset = 0;
	int trivialDataSize = 0;
	CArray<CDestructibleUtilityData> destructibleUtilityData;
	CMemoryOwner<> destructibleData;
	int destructibleDataOffset = 0;
	int destructibleDataSize = 0;

	void callDestructors();
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for adding initial components to an entity.
class REAPI CEntityInitializer {
public:
	explicit CEntityInitializer( CEntityInitializationData& _initData, CFullEntityData& _emptyEntity );

	// Get a reference that can be used to access an entity that will be created using this initializer.
	CEntityRef GetFutureReference() const
		{ return CEntityRef( &emptyEntity, emptyEntity.Generation ); }
	// Get the current value of the given component. Null is returned if no data is present.
	// This method performs linear search to find the data.
	template <class T>
	const T* FindComponentData( const CComponent<T>& comp ) const;

	template <class CompType, class... Args>
	void AddComponentData( const CComponent<CompType>& comp, Args&&... data );

	// Container needs access to internal structures for finalizing entity filling.
	friend class CEntityContainer;

private:
	// Groupless entity that will be initialized with this initializer.
	CFullEntityData& emptyEntity;
	CEntityInitializationData& initData;

	template <class T>
	static bool isTNonTrivial();

	void growDestructibleData( int minSize );
	void growTrivialData( int minSize );
};

//////////////////////////////////////////////////////////////////////////

template <class T>
const T* CEntityInitializer::FindComponentData( const CComponent<T>& comp ) const
{
	const auto compId = comp.GetComponentId();
	if( isTNonTrivial<T>() ) {
		const auto basePtr = reinterpret_cast<BYTE*>( initData.destructibleData.Ptr() );
		for( int i = initData.destructibleUtilityData.Size() - 1; i >= 0; i-- ) {
			if( initData.destructibleUtilityData[i].CompId == compId ) {
				return reinterpret_cast<T*>( basePtr + initData.destructibleUtilityData[i].Offset );
			}
		}
		return nullptr;
	} else {
		const auto basePtr = reinterpret_cast<BYTE*>( initData.trivialData.Ptr() );
		for( int i = initData.trivialUtilityData.Size() - 1; i >= 0; i-- ) {
			if( initData.trivialUtilityData[i].CompId == compId ) {
				return reinterpret_cast<T*>( basePtr + initData.trivialUtilityData[i].Offset );
			}
		}
		return nullptr;
	}
}

template <class CompType, class... Args>
void CEntityInitializer::AddComponentData( const CComponent<CompType>& comp, Args&&... data )
{
	initData.components.Add( comp );
	CompType convertedData( forward<Args>( data )... );
	if( isTNonTrivial<CompType>() ) {
		const auto offset = initData.destructibleDataOffset;
		const int newOffset = sizeof( CompType ) + offset;
		if( newOffset > initData.destructibleDataSize ) {
			growDestructibleData( newOffset );
		}
		initData.destructibleUtilityData.Add( comp.GetDestroyFunction(), comp.GetMoveConstructFunction(), offset, comp.GetComponentId() );
		const auto moveFunctionPtr = comp.GetMoveConstructFunction();
		const auto basePtr = reinterpret_cast<BYTE*>( initData.destructibleData.Ptr() );
		moveFunctionPtr( &convertedData, basePtr + offset, sizeof( CompType ) );
		initData.destructibleDataOffset = newOffset;
	} else {
		const auto offset = initData.trivialDataOffset;
		const int newOffset = sizeof( CompType ) + offset;
		if( newOffset > initData.trivialDataSize ) {
			growTrivialData( newOffset );
		}
		initData.trivialUtilityData.Add( offset, comp.GetComponentId() );
		const auto basePtr = reinterpret_cast<BYTE*>( initData.trivialData.Ptr() );
		::memcpy( basePtr + offset, &convertedData, sizeof( CompType ) );
		initData.trivialDataOffset = newOffset;
	}
}

template <class T>
bool CEntityInitializer::isTNonTrivial()
{
	return CComponent<T>::IsTNonTrivial();
}

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

struct CComponentInitializationData {
	int Id;
	void* Data;
};

//////////////////////////////////////////////////////////////////////////

// A structural storage for component data. Filled with arbitrary components and is passed to an entity container to efficiently initialize an entity.
class REAPI CFilledEntityData {
public:
	explicit CFilledEntityData( CEntityInitializationData& _initData ) : initData( _initData ) {}

	const CComponentGroup& GetComponentGroup() const
		{ return initData.components; }

	int GetTrivialComponentCount() const
		{ return initData.trivialUtilityData.Size(); }
	CComponentInitializationData GetTrivialData( int index );

	int GetDestructibleComponentCount() const
		{ return initData.destructibleUtilityData.Size(); }
	CComponentInitializationData GetDestructibleData( int index );

private:
	CEntityInitializationData& initData;
};

//////////////////////////////////////////////////////////////////////////

inline CComponentInitializationData CFilledEntityData::GetTrivialData( int index )
{
	const auto basePtr = reinterpret_cast<BYTE*>( initData.trivialData.Ptr() );
	const auto utilityData = initData.trivialUtilityData[index];
	return CComponentInitializationData{ utilityData.CompId, basePtr + utilityData.Offset };
}

inline CComponentInitializationData CFilledEntityData::GetDestructibleData( int index )
{
	const auto basePtr = reinterpret_cast<BYTE*>( initData.destructibleData.Ptr() );
	const auto utilityData = initData.destructibleUtilityData[index];
	return CComponentInitializationData{ utilityData.CompId, basePtr + utilityData.Offset };
}

//////////////////////////////////////////////////////////////////////////

} // namespace RelibInternal.

} // namespace Relib.

