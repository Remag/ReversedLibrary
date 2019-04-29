#include <EntityInitializer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

void CEntityInitializationData::callDestructors()
{
	const auto basePtr = reinterpret_cast<BYTE*>( destructibleData.Ptr() );
	const auto destructibleCompCount = destructibleUtilityData.Size();
	if( destructibleCompCount == 0 ) {
		return;
	}

	for( int i = 0; i < destructibleCompCount - 1; i++ ) {
		const auto currentOffset = destructibleUtilityData[i].Offset;
		const auto dataSize = destructibleUtilityData[i + 1].Offset - currentOffset;
		destructibleUtilityData[i].DestroyPtr( basePtr + currentOffset, dataSize );
	}
	auto& lastData = destructibleUtilityData.Last();
	const auto lastSize = destructibleDataOffset - lastData.Offset;
	lastData.DestroyPtr( basePtr + lastData.Offset, lastSize );
}

//////////////////////////////////////////////////////////////////////////

CEntityInitializer::CEntityInitializer( CEntityInitializationData& _initData, CFullEntityData& _emptyEntity ) :
	initData( _initData ),
	emptyEntity( _emptyEntity )
{
	initData.callDestructors();
	initData.components.Empty();
	initData.trivialUtilityData.Empty();
	initData.destructibleUtilityData.Empty();
	initData.trivialDataOffset = 0;
	initData.destructibleDataOffset = 0;
}

void CEntityInitializer::growDestructibleData( int minSize )
{
	const auto newSize = CDefaultGrowStrategy<8>::GrowValue( initData.destructibleDataSize, minSize );
	CMemoryOwner<> newBuffer{ CRuntimeHeap::Allocate( newSize ) };

	const auto baseDestPtr = reinterpret_cast<BYTE*>( initData.destructibleData.Ptr() );
	const auto baseSrcPtr = reinterpret_cast<BYTE*>( newBuffer.Ptr() );
	const auto destructibleCompCount = initData.destructibleUtilityData.Size();
	if( destructibleCompCount != 0 ) {
		for( int i = 0; i < destructibleCompCount - 1; i++ ) {
			const auto currentOffset = initData.destructibleUtilityData[i].Offset;
			const auto dataSize = initData.destructibleUtilityData[i + 1].Offset - currentOffset;
			initData.destructibleUtilityData[i].MovePtr( baseDestPtr + currentOffset, baseSrcPtr + currentOffset, dataSize );
		}
		auto& lastData = initData.destructibleUtilityData.Last();
		const auto lastSize = initData.destructibleDataOffset - lastData.Offset;
		lastData.MovePtr( baseDestPtr + lastData.Offset, baseSrcPtr + lastData.Offset, lastSize );
	}

	initData.destructibleData = move( newBuffer );
	initData.destructibleDataSize = newSize;
}

void CEntityInitializer::growTrivialData( int minSize )
{
	const auto newSize = CDefaultGrowStrategy<8>::GrowValue( initData.trivialDataSize, minSize );
	CMemoryOwner<> newBuffer{ CRuntimeHeap::Allocate( newSize ) };
	memcpy( newBuffer.Ptr(), initData.trivialData.Ptr(), initData.trivialDataSize );
	initData.trivialData = move( newBuffer );
	initData.trivialDataSize = newSize;
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.
