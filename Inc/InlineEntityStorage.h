#pragma once
#include <Array.h>
#include <ArrayBuffer.h>
#include <StackArray.h>
#include <StaticArray.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Data necessary for component cleanup. Most components are assumed to not require cleanup.
struct CStorageCleanupData {
	typedef void ( *TCleanupFunction )( CStaticArray<BYTE>& storage, int offset );
	typedef void ( *TMoveFunction )( CStaticArray<BYTE>& src, CStaticArray<BYTE>& dest, int offset );

	int Offset;
	TCleanupFunction CleanupFunction;
	TMoveFunction MoveFunction;

	CStorageCleanupData( int offset, TCleanupFunction cleanupFunction, TMoveFunction moveFunction ) :
		Offset( offset ), CleanupFunction( cleanupFunction ), MoveFunction( moveFunction ) {}
};

//////////////////////////////////////////////////////////////////////////

struct CEntityIndexData {
	short ComponentId;
	unsigned short OffsetData;
};

struct CInlineEntityStorage {
	// Component binary data.
	CStaticArray<BYTE> ComponentData;
	// Relation between component identifiers and their offset in storage.
	// The index is stored as a compact hash table.
	CStackArray<CEntityIndexData, 32> OffsetIndex;
	CArray<CEntityIndexData> OffsetOverflowData;
	// Current offset that will be used for adding a new component to storage.
	int CurrentOffset = 0;
	// Registered component cleanup functions.
	CArray<CStorageCleanupData> CleanupFunctions;

	CInlineEntityStorage();
	~CInlineEntityStorage();
	// Copying is prohibited.
	CInlineEntityStorage( CInlineEntityStorage& ) = delete;
	void operator=( CInlineEntityStorage& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

inline CInlineEntityStorage::CInlineEntityStorage()
{
	for( auto& data : OffsetIndex ) {
		data.ComponentId = NotFound; 
	}
}

inline CInlineEntityStorage::~CInlineEntityStorage()
{
	for( auto& cleanupData : CleanupFunctions ) {
		cleanupData.CleanupFunction( ComponentData, cleanupData.Offset );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

