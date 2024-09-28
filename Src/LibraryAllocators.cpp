#include <LibraryAllocators.h>
#include <Reutils.h>
#include <StaticAllocators.h>
#include <RawBuffer.h>

namespace Relib {

namespace RelibInternal {

extern CVirtualAllocDynamicManager VirtualMemoryAllocator;
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG 

CVirtualAllocDynamicManager::CVirtualAllocDynamicManager() :
	firstBlock( nullptr )
{
}

CVirtualAllocDynamicManager::~CVirtualAllocDynamicManager()
{
	CCriticalSectionLock lock( virtualAllocatorLock );
	for( CVirtualMemoryDebugBlock* block = firstBlock; block != nullptr; block = block->Next() ) {
		block->CheckFree();
	}
}

void* CVirtualAllocDynamicManager::Allocate( int size )
{
	return Allocate( size, "", 0 );
}

CRawBuffer CVirtualAllocDynamicManager::AllocateSized( int size, const char* fileName, int line )
{
	return { Allocate( size, fileName, line ), size };
}

void CVirtualAllocDynamicManager::Free( void* ptr )
{
	CCriticalSectionLock lock( virtualAllocatorLock );
	CVirtualMemoryDebugBlock* block = CVirtualMemoryDebugBlock::CreateFromData( ptr );
	if( block == firstBlock ) {
		firstBlock = block->Next();
	}
	block->SetFree();
	const BOOL result = ::VirtualFree( block, 0, MEM_RELEASE );
	assert( result != 0 );
}

void* CVirtualAllocDynamicManager::Allocate( int size, const char* fileName, int line )
{
	const int blockSize = CVirtualMemoryDebugBlock::DebugBlockSize( size );
	void* ptr = ::VirtualAlloc( 0, blockSize, MEM_COMMIT, PAGE_READWRITE );
	checkMemoryError( ptr != nullptr );
	CCriticalSectionLock lock( virtualAllocatorLock );
	firstBlock = ::new( ptr ) CVirtualMemoryDebugBlock( firstBlock, size, fileName, line );
	return firstBlock->Data();
}

#else

CVirtualAllocDynamicManager::CVirtualAllocDynamicManager()
{
}

CVirtualAllocDynamicManager::~CVirtualAllocDynamicManager()
{
}

void* CVirtualAllocDynamicManager::Allocate( int size )
{
	const auto result = ::VirtualAlloc( 0, size, MEM_COMMIT, PAGE_READWRITE );
	checkMemoryError( result != nullptr );
	return result;
}

void CVirtualAllocDynamicManager::Free( void* ptr )
{
	const auto result = ::VirtualFree( ptr, 0, MEM_RELEASE );
	result;
	assert( result != 0 );
}

#endif

CRawBuffer CVirtualAllocDynamicManager::AllocateSized( int size )
{
	return { Allocate( size ), size };
}

void CVirtualAllocDynamicManager::Free( CRawBuffer buffer )
{
	Free( buffer.Ptr() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.
