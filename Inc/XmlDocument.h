#pragma once
#include <Errors.h>
#include <RapidXml\rapidxml.hpp>
// Tools for reading and editing XML documents. Wraps RapidXML.

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Exception thrown when a parsing error occurs.
class REAPI CXmlException : public CException {
public:
	CXmlException( CUnicodePart description, int symbolPos );
	CXmlException( const CXmlException& other ) : description( copy( other.description ) ), symbolPos( other.symbolPos ) {}

	virtual CUnicodeString GetMessageText() const override;

private:
	const CUnicodeString description;
	const int symbolPos;
};

//////////////////////////////////////////////////////////////////////////

class CXmlElement;

// An XML document. Parses xml files and allocates memory for elements.
// Document owns all elements that are attached to it.
// It is recommended for performance to create CXmlDocuments on the stack.
class REAPI CXmlDocument {
public:
	CXmlDocument();

	CUnicodeView GetName() const
		{ return sourceStrName; }

	// Saving and loading.
	void LoadFromFile( CUnicodePart fileName );
	void LoadFromString( CUnicodeString str );
	void SaveToFile( CUnicodeView fileName ) const;

	bool HasRoot() const
		{ return root != nullptr; }
	// Get the root element. It must exist.
	const CXmlElement& GetRoot() const
		{ assert( root != nullptr ); return *root; }
	CXmlElement& GetRoot()
		{ assert( root != nullptr ); return *root; }

	// Set the new root element.
	void SetRoot( CUnicodeView name );
	// Set the new root.
	// Previous root element is copied to the document's allocator if necessary.
	void SetRoot( CXmlElement& elem );

	// Create an empty element on this document.
	CXmlElement& CreateXmlElement( CUnicodePart name );

	// Clear the root element.
	void Empty();

	// Element needs access to allocation and copying.
	friend class CXmlElement;

private:
	// Name of the document string source. Corresponds with file names for documents created with files.
	// Can be used to identify the document for diagnostic purposes.
	CUnicodeString sourceStrName;
	// Root element.
	CXmlElement* root;
	// rapidXML class that is used for parsing and allocating elements.
	rapidxml::xml_document document;

	CUnicodeView createName( CUnicodeString nameSrc );
	CXmlElement* createElement( CUnicodeString name );
	CXmlElement* copyElement( const CXmlElement& element );

	// Copying is prohibited.
	CXmlDocument( CXmlDocument& ) = delete;
	void operator=( CXmlDocument& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

