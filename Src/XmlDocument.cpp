#include <XmlDocument.h>
#include <XmlElement.h>
#include <FileMapping.h>
#include <StrConversions.h>

namespace Relib {

extern const CStringView XmlParsingError;
CXmlException::CXmlException( CStringPart _description, long long _symbolPos ) :
	description( _description ),
	symbolPos( _symbolPos )
{
}

CString CXmlException::GetMessageText() const 
{
	return XmlParsingError.SubstParam( symbolPos, description );
}

//////////////////////////////////////////////////////////////////////////

CXmlDocument::CXmlDocument() :
	document( *this ),
	root( nullptr )
{
}

void CXmlDocument::LoadFromFile( CStringPart fileName )
{
	document.Empty();
	sourceStrName = Str( fileName );
	root = document.parse( File::ReadUnicodeText( sourceStrName ) );
}

extern const CStringView CreatedFromStrName;
void CXmlDocument::LoadFromString( CUnicodeString str )
{
	document.Empty();
	sourceStrName = Str( CreatedFromStrName );
	root = document.parse( move( str ) );
}

static const CUnicodeView xmlHeaderText = L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
void CXmlDocument::SaveToFile( CStringPart fileName ) const
{
	if( root == nullptr ) {
		return;
	}
	const auto xmlText = xmlHeaderText + root->ToString();
	File::WriteUnicodeText( fileName, xmlText, FTE_UTF8 );
}

void CXmlDocument::SetRoot( CUnicodeView name )
{
	CXmlElement* newElem = createElement( UnicodeStr( name ) );
	root = newElem;
}

void CXmlDocument::SetRoot( CXmlElement& elem )
{
	if( &elem.GetDocument() == this ) {
		// Memory for all the upper nodes will not be freed until the whole document is cleared.
		elem.Detach();
		root = &elem;
	} else {
		// Elem is from another document, we have to copy it to ours.
		elem.Detach();
		root = copyElement( elem );
		
	}
}

CXmlElement& CXmlDocument::CreateXmlElement( CUnicodePart name )
{
	return *createElement( UnicodeStr( name ) );
}

void CXmlDocument::Empty()
{
	root = nullptr;
}

// Create a string on the document's allocator.
CUnicodeView CXmlDocument::createName( CUnicodeString nameSrc )
{
	return document.AllocateString( move( nameSrc ) );
}

// Allocate memory for the element on the document's pool and construct the element.
CXmlElement* CXmlDocument::createElement( CUnicodeString name )
{
	return document.AllocateElement( move( name ) );
}

// Create a copy on the document's pool.
CXmlElement* CXmlDocument::copyElement( const CXmlElement& element )
{
	return document.CreateCopy( element );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.