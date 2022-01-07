#pragma once

// Standard library definitions.
#include <new.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <WinSock2.h>
#include <windows.h>
#include <typeinfo>
#include <limits.h>
#include <intrin.h>
#include <locale.h>
#include <float.h>
#include <initializer_list>

#ifdef RELIB_MINIMAL
#define RELIB_NO_INTERNET
#define RELIB_NO_IMAGELIB
#endif

// Disable the CRT provided assertion.
#define NDEBUG
#include <atomic>
#undef NDEBUG
#undef assert

// Definitions for unicode strings for source file name and function name.
// Taken from MSDN.
#define __WIDEN2( x ) L##x
#define __WIDEN( x ) __WIDEN2( x )
#define __UNICODEFILE__ __WIDEN( __FILE__ )
#define __UNICODEFUNCTION__ __WIDEN( __FUNCTION__ )

// Include dependency libraries for projects using Relib.

#ifdef _WIN64
#define PLATFORM_SUFFIX "64"
#else
#define PLATFORM_SUFFIX ""
#endif

#define RELIB_SUFFIX PLATFORM_SUFFIX

#if defined( REBUILD_DYNAMIC )
#define REAPI __declspec( dllexport )
#elif defined( REBUILD ) || defined( USE_STATIC_RELIB )
#define REAPI
#else
#define REAPI __declspec( dllimport )
#endif

#ifndef REBUILD
#pragma comment( lib, "ReversedLibrary" RELIB_SUFFIX ".lib" )
#endif

#if defined( REBUILD ) || defined( USE_STATIC_RELIB )
// Relib depends on a number of external libraries.
#pragma comment( lib, "FreeType.lib" )

#ifndef RELIB_NO_IMAGELIB
#pragma comment( lib, "jpeg.lib" )
#pragma comment( lib, "libpng16.lib" )
#pragma comment( lib, "zlib.lib" )
#endif

#ifndef RELIB_NO_INTERNET
#pragma comment( lib, "libcurl.lib" )
#pragma comment( lib, "zlib.lib" )
#endif

#endif

// Some dependencies come from a static build of curl and are only required for static builds.
#ifdef USE_STATIC_RELIB
#ifndef RELIB_NO_INTERNET
#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "wldap32.lib" )
#pragma comment( lib, "crypt32.lib" )
#endif
#endif

namespace Relib {

namespace RelibInternal {
	template <class T>
	class CArrayConstData;
	template <class T>
	class CStringData;
	template <class T>
	class CBaseStringPart;
	template <class T>
	class CBaseStringView;
	template <class T>
	class CBaseString;
	template <class T>
	class CRawStringBuffer;
	class CUnicodeSetAllocator;
	template <class T>
	struct CIndexEntry;
	template <class ContainedType, class HashStrategy, class Allocator>
	class CHashIndex;
	template <class T, int objectGroupSize, class GroupAllocator, class GeneralAllocator>
	class CObjectPool;
	struct CInlineEntityStorage;
	class CFilledEntityData;
}
	enum TFileCreationMode : unsigned;
	enum TFileReadWriteMode : unsigned;
	enum TFileShareMode : unsigned;

	typedef RelibInternal::CBaseStringPart<char> CStringPart;
	typedef RelibInternal::CBaseStringPart<wchar_t> CUnicodePart;
	typedef RelibInternal::CBaseStringView<char> CStringView;
	typedef RelibInternal::CBaseStringView<wchar_t> CUnicodeView;
	typedef RelibInternal::CBaseString<char> CString;
	typedef RelibInternal::CBaseString<wchar_t> CUnicodeString;	
	typedef RelibInternal::CRawStringBuffer<char> CStringBuffer;
	typedef RelibInternal::CRawStringBuffer<wchar_t> CUnicodeBuffer;	

	class ISerializable;
	class IExternalObject;
	struct CFileStatus;
	class CFileReadView;
	class CFileWriteView;
	class CFileReadWriteView;
	class CFileReader;
	class CFileWriter;
	class CFileReadWriter;
	class CDynamicFile;
	class CProcessHeap;
	class CRuntimeHeap;
	class CHeapAllocator;
	class CArchiveReader;
	class CArchiveWriter;
	class CError;
	class CInternalException;
	class CException;
	class CFileException;
	class CCheckException;
	class CMemoryException;
	class CXmlException;
	class CXmlDocument;
	class CXmlElement;
	class CXmlAttribute;
	class CCriticalSection;
	class CFontView;
	class CFontEdit;
	class CFontOwner;
	class CFontSizeView;
	class CFontSizeOwner;
	class CGlyph;
	class CRawBuffer;
	class CExplicitCopyTag;
	class CMessageSource;
	class CInternetFile;
	class CInternetFileBatch;
	class CWebConnectionScheduler;
	class CRegistryKey;
	class CRegistryKeyValueEnumerator;
	class CReadWriteSection;
	class CReadLock;
	class CWriteLock;
	struct CColor;

	class CEntityGroup;
	class CEntity;
	class CComponentGroup;
	class CEntityComponentSystem;

	template <class T, class ComponentClass>
	class CInlineComponent;

	template <class Allocator>
	class CThreadSafeAllocator;

	template <int minValue>
	class CDefaultGrowStrategy;
	template <int minValue>
	class CFlexibleArrayGrowStrategy;
	template <class Allocator, int stackSize, int alignment>
	class CInlineStackAllocator;

	template <class Elem, class Allocator = CRuntimeHeap, class GrowStrategy = CDefaultGrowStrategy<8>>
	class CArray;
	template <class EnumType, class ValueType = CUnicodeString>
	struct CEnumItem;
	template <class EnumType, int enumSize, class ValueType = CUnicodeString>
	class CEnumDictionary;
	template <class T>
	class CDefaultHash;
	template <class T>
	class IAction;
	template <class T>
	class COptional;
	template <class Elem, class Allocator = CRuntimeHeap>
	class CStaticArray;
	template <class Elem, int stackSize>
	class CStackArray;
	template <class Elem>
	class CArrayView;
	template <class Elem>
	class CArrayBuffer;
	template <class T, class Allocator = CRuntimeHeap>
	class CPtrOwner;
	template <class T, class Allocator = CRuntimeHeap>
	class CSharedPtr;
	template <class Type, int dim>
	class CVector;
	template <class FirstType, class SecondType = FirstType>
	struct CPair;
	template <class T>
	class CAtomic;
	template <class Key, class Value, class HashStrategy, class Allocator>
	class CMap;
	template <class... Types>
	class CTuple;
	template <class Callable>
	class CActionOwner;
	template <class Callable>
	class CMutableActionOwner;
	template <class T>
	class CFuture;
	template <class T>
	class CPromise;

	const int NotFound = -1;
} // namespace Relib.

#pragma warning( disable : 4251 )	// class A needs to have dll-interface to be used by clients of class B.	[ fires on templated classes with available source ]
#pragma warning( disable : 4127 )	// conditional expression is constant.	[ useful in templates ]
#pragma warning( disable : 4512 )	// assignment operator could not be generated.	[ good thing we don't use it anywhere ]
#pragma warning( disable : 4324 )	// structure was padded due to alignment specifier.	[ that's the idea ]
#pragma warning( disable : 4702 )	// unreachable code.	[ false positives after optimizations ]
#pragma warning( disable : 4505 )	// unreferenced local function has been removed. [ good ]
#pragma warning( disable : 4592 )	// symbol will be dynamically initialized (implementation limitation). [ this warning is a bug in VS2015 Update 1 ].
#pragma warning( disable : 4521 )	// multiple copy constructors specified. [ used for correct overload resolution in COptional ].
#pragma warning( disable : 4471 )	// a forward declaration of an unscoped enumeration must have an underlying type.
#pragma warning( disable : 5045 )	// compiler will insert Spectre mitigation for memory load if /Qspectre switch specified.
#pragma warning( disable : 26812 )	// The enum type 'TType' is unscoped. Prefer 'enum class' over 'enum'

// Library defines its own min and max functions.
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace Relib;