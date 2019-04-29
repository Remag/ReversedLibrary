#include <ObjectCreation.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <Map.h>
#include <CriticalSection.h>
#include <PtrOwner.h>
// typeid silently returns external object's static type without this include. Do not delete.
#include <ExternalObject.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern CCriticalSection ObjectCreationFunctionsSection;
// Maps connecting object's creation function with its external name.
extern CMap<CUnicodeString, CPtrOwner<CBaseObjectCreationFunction, CProcessHeap>, CDefaultHash<CUnicodeString>, CProcessHeap> ObjectCreationFunctions;

// Map connecting object's name with its type_info::name() pointer.
extern CMap<CStringView, CUnicodeString, CDefaultHash<CStringView>, CProcessHeap> ObjectRegisteredNames;

const CBaseObjectCreationFunction* GetObjectCreationFunction( CUnicodePart objectName )
{
	CCriticalSectionLock lock( ObjectCreationFunctionsSection );
	assert( ObjectCreationFunctions.Has( objectName ) );
	return ObjectCreationFunctions[objectName];
}

void RegisterObject( const type_info& objectInfo, CUnicodePart objectName, CPtrOwner<CBaseObjectCreationFunction, CProcessHeap> newFunction )
{
	CCriticalSectionLock lock( ObjectCreationFunctionsSection );
	assert( !ObjectCreationFunctions.Has( objectName ) );
	ObjectCreationFunctions.Set( objectName, move( newFunction ) );
	ObjectRegisteredNames.Set( objectInfo.name(), objectName );
}

CUnicodeView GetExternalName( const IExternalObject& object )
{
	const char* const objectName = typeid( object ).name();
	CCriticalSectionLock lock( ObjectCreationFunctionsSection );
	return ObjectRegisteredNames[objectName];
}

bool IsExternalName( CUnicodePart name )
{
	CCriticalSectionLock lock( ObjectCreationFunctionsSection );
	return ObjectCreationFunctions.Has( name );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
