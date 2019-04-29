#include <RapidXml\rapidxml.hpp>
#include <Redefs.h>
#include <StaticAllocators.h>
#include <XmlDocument.h>
#include <XmlElement.h>
#include <StrConversions.h>
// RapidXML methods that were repositioned here to resolve include conflicts.

//////////////////////////////////////////////////////////////////////////

// RapidXml original header:

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $

//////////////////////////////////////////////////////////////////////////

namespace rapidxml {

//////////////////////////////////////////////////////////////////////////

xml_document::xml_document( CXmlDocument& _owner, int _flags /*= parse_default */ ) : 
	owner( _owner ),
	flags( _flags ),
	textStart( 0 )
 {
 }

xml_document::xml_document( xml_document&& other ) = default;

xml_document::~xml_document()
{
}

CXmlElement* xml_document::CreateCopy( const CXmlElement& elem )
{
	CUnicodeView internalName = AllocateString( UnicodeStr( elem.Name() ) );
	CXmlElement& result = elementStorage.Add( internalName, owner, CElementCreationKey() );
	result.SetText( UnicodeStr( elem.GetText() ) );
	for( const CXmlElement* child = elem.FirstChild(); child != nullptr; child = child->Next() ) {
		result.attachLastChild( CreateCopy( *child ) );
	}
	for( const auto& attribute : elem.Attributes() ) {
		result.AddAttribute( UnicodeStr( attribute.Name() ), UnicodeStr( attribute.GetValueText() ) );
	}
	return &result;
}

//////////////////////////////////////////////////////////////////////////

CXmlElement* xml_document::parse( CUnicodeString str )
{
	documentContent.Empty();
	const wchar_t* text = str.Ptr();
	documentContent.Add( move( str ) );
	assert( text != nullptr );
	textStart = text;

	// Parse children
	while( true ) {
		// Skip whitespace before node.
		skip<whitespace_pred>(text);
		if( *text == 0 ) {
			break;
		}

		// Parse and append new child.
		if( *text == L'<' ) {
			++text;     // Skip '<'
			if( CXmlElement* element = parse_node( text ) ) {
				// parse_node will not return null only if root has been parsed.
				return element;
			}
		} else {
			throw CXmlException( L"expected <", text - textStart );
		}
	}
	// No root detected. Clear the content and leave.
	documentContent.Empty();
	return nullptr;
}

void xml_document::skip_xml_declaration( const wchar_t*& text )
{
	// Skip until end of declaration.
	while (text[0] != L'?' || text[1] != L'>') {
		if (!text[0]) {
			throw CXmlException( L"unexpected end of data", text - textStart );
		}
		++text;
	}
	text += 2;    // Skip '?>'
}

void xml_document::skip_comment( const wchar_t*& text )
{
	// Skip until end of comment.
	while( text[0] != L'-' || text[1] != L'-' || text[2] != L'>' ) {
		if( !text[0] ) {
			throw CXmlException( L"unexpected end of data", text - textStart );
		}
		++text;
	}
	text += 3;     // Skip '-->'
	return;
}

void xml_document::skip_doctype( const wchar_t*& text )
{
	// Skip to >
	while( *text != L'>' ) {
		// Determine character type.
		switch (*text)
		{
				
		// If '[' encountered, scan for matching ending ']' using naive algorithm with depth.
		// This works for all W3C test files except for 2 most wicked.
		case L'[':
		{
			++text;     // Skip '['
			int depth = 1;
			while (depth > 0)
			{
				switch (*text)
				{
					case L'[': ++depth; break;
					case L']': --depth; break;
					case 0: throw CXmlException( L"unexpected end of data", text - textStart );
				}
				++text;
			}
			break;
		}
				
		// Error on end of text.
		case L'\0':
			throw CXmlException( L"unexpected end of data", text - textStart );
				
		// Other character, skip it.
		default:
			++text;
		}
	}
	text += 1;      // skip '>'
	return;
}

void xml_document::skip_pi( const wchar_t*& text )
{
	// Skip to '?>'
	while (text[0] != L'?' || text[1] != L'>') {
		if( *text == L'\0' ) {
			throw CXmlException( L"unexpected end of data", text - textStart );
		}
		++text;
	}
	text += 2;    // Skip '?>'
}

void xml_document::parse_and_append_data( CXmlElement& element, const wchar_t*& text )
{
	// Skip until end of data.
	const wchar_t* value = text;
	skip<text_pred>( text );
			
	// Add data to parent node if no data exists yet.
	if ( !(flags & parse_no_element_values) && element.GetText().IsEmpty() ) {
		// Get rid of the trailing whitespace.
		const CUnicodePart elementText( CUnicodePart( value, text - value ).TrimSpaces() );
		element.text = elementText;
	}
}

void xml_document::skip_cdata( const wchar_t*& text )
{
	// Skip until end of cdata.
	while( text[0] != L']' || text[1] != L']' || text[2] != L'>' ) {
		if( !text[0] ) {
			throw CXmlException( L"unexpected end of data", text - textStart );
		}
		++text;
	}
	text += 3;      // Skip ]]>
}

CXmlElement* xml_document::parse_element( const wchar_t*& text )
{
	// Extract element name
	const wchar_t* name = text;
	skip<node_name_pred>(text);
	if( text == name ) {
		throw CXmlException( L"expected element name", text - textStart );
	}

	const CUnicodePart elemName = CUnicodePart( name, text - name );
	// Create element node
	CXmlElement* element = &elementStorage.Add( elemName, owner, CElementCreationKey() );

	// Skip whitespace between element name and attributes or >.
	skip<whitespace_pred>( text );

	// Parse attributes, if any.
	parse_node_attributes( text, *element );

	// Determine ending type.
	if( *text == L'>' ) {
		++text;
		parse_node_contents( text, *element );
	}
	else if( *text == L'/' ) {
		++text;
		if( *text != L'>' ) {
			throw CXmlException( L"expected >", text - textStart );
		}
		++text;
	}
	else {
		throw CXmlException( L"expected >", text - textStart );
	}

	// Return parsed element.
	return element;
}

CXmlElement* xml_document::parse_node( const wchar_t*& text )
{
	// Parse proper node type.
	switch (text[0])
	{

	// <...
	default: 
		// Parse and append element node.
		return parse_element(text);

	// <?...
	case L'?': 
		++text;     // Skip ?
		if ( (text[0] == L'x' || text[0] == L'X' ) &&
			( text[1] == L'm' || text[1] == L'M' ) && 
			( text[2] == L'l' || text[2] == L'L' ) &&
			whitespace_pred::test(text[3]))
		{
			// '<?xml ' - xml declaration
			text += 4;      // Skip 'xml '
			skip_xml_declaration(text);
			return 0;
		}
		else {
			// Skip PI.
			skip_pi(text);
			return 0;
		}
			
	// <!...
	case L'!': 

		// Parse proper subset of <! node
		switch (text[1])    
		{
				
		// <!-
		case L'-':
			if( text[2] == L'-' ) {
				// '<!--' - xml comment
				text += 3;     // Skip '!--'
				skip_comment(text);
				return 0;
			}
			break;

		// <![
		case L'[':
			if( text[2] == L'C' && text[3] == L'D' && text[4] == L'A' && 
				text[5] == L'T' && text[6] == L'A' && text[7] == L'[' )
			{
				// '<![CDATA[' - cdata
				text += 8;     // Skip '![CDATA['
				skip_cdata(text);
				return 0;
			}
			break;

		// <!D
		case L'D':
			if (text[2] == L'O' && text[3] == L'C' && text[4] == L'T' && 
				text[5] == L'Y' && text[6] == L'P' && text[7] == L'E' && 
				whitespace_pred::test(text[8]))
			{
				// '<!DOCTYPE ' - doctype
				text += 9;      // skip '!DOCTYPE '
				skip_doctype(text);
				return 0;
			}

		}   // switch

		// Attempt to skip other, unrecognized node types starting with <!.
		++text;     // Skip !
		while( *text != L'>' ) {
			if( *text == 0 ) {
				throw CXmlException( L"unexpected end of data", text - textStart );
			}
			++text;
		}
		++text;     // Skip '>'
		return 0;   // No node recognized
	}
}

void xml_document::parse_node_contents( const wchar_t*& text, CXmlElement& element )
{
	// For all children and text.
	while( true ) {
		// Skip whitespace between > and node contents.
		skip<whitespace_pred>( text );

		// Determine what comes next: node closing, child node, data node, or 0?
		switch( *text ) {
		// Node closing or child node.
		case L'<':
			if( text[1] == L'/' ) {
				// Node closing
				text += 2;      // Skip '</'
				// No validation, just skip name.
				skip<node_name_pred>( text );
				// Skip remaining whitespace after node name.
				skip<whitespace_pred>( text );
				if( *text != '>' ) {
					throw CXmlException( L"expected >", text - textStart );
				}
				++text;     // Skip '>'
				return;     // Node closed, finished parsing contents.
			} else {
				// charild node.
				++text;     // Skip '<'
				if( CXmlElement* child = parse_node( text ) ) {
					element.attachLastChild( child );
				}
			}
			break;

		// End of data - error.
		case L'\0':
			throw CXmlException( L"unexpected end of data", text - textStart );

		// Data node.
		default:
			parse_and_append_data( element, text );
		}
	}
}

void xml_document::parse_node_attributes( const wchar_t*& text, CXmlElement& elem )
{
	// For all attributes.
	while( attribute_name_pred::test( *text ) ) {
		// Extract attribute name.
		const wchar_t* name = text;
		++text;     // Skip first character of attribute name.
		skip<attribute_name_pred>( text );
		if( text == name ) {
			throw CXmlException( L"expected attribute name", text - textStart );
		}

		// Create new attribute.
		const CUnicodePart attrName{ name, text - name };

		// Skip whitespace after attribute name.
		skip<whitespace_pred>(text);

		// Skip =
		if( *text != L'=' )
			throw CXmlException( L"expected =", text - textStart );
		++text;

		// Skip whitespace after =
		skip<whitespace_pred>(text);

		// Skip quote and remember if it was ' or "
		wchar_t quote = *text;
		if( quote != L'\'' && quote != L'"' ) {
			throw CXmlException( L"expected ' or \"", text - textStart );
		}
		++text;

		// Extract attribute value.
		const wchar_t* value = text;
		if ( quote == L'\'' ) {
			skip<attribute_value_pred_single>( text );
		} else {
			skip<attribute_value_pred_double>( text );
		}
		// Set attribute value.
		const CUnicodePart attrValue{ value, text - value };
		elem.attributes.Add( attrName, attrValue );
				
		// Make sure that end quote is present.
		if( *text != quote ) {
			throw CXmlException( L"expected ' or \"", text - textStart );
		}
		// Skip quote.
		++text;    
		// Skip whitespace after attribute value.
		skip<whitespace_pred>(text);
	}
}

CXmlElement* xml_document::AllocateElement( CUnicodeString name )
{
	const CUnicodeView internalName = AllocateString( move( name ) );
	return &elementStorage.Add( internalName, owner, CElementCreationKey() );
}

CUnicodeView xml_document::AllocateString( CUnicodeString name )
{
	return documentContent.Add( move( name ) );
}

void xml_document::Empty()
{
	elementStorage.Empty();
	documentContent.Empty();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace rapidxml

