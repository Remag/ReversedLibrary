#pragma once
// RapidXml source code.
// This code has been modified to create a higher level structure while parsing.
// Most non-basic functionality has been cut.
// Encoding attribute of the file is ignored

//////////////////////////////////////////////////////////////////////////

// RapidXml original header:

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $
//! \file rapidxml.hpp This file contains rapidxml parser and DOM implementation

//////////////////////////////////////////////////////////////////////////

#include <Redefs.h>
#include <BaseString.h>
#include <Array.h>
#include <HashTable.h>
#include <PersistentStorage.h>
#include <UnicodeSet.h>

///////////////////////////////////////////////////////////////////////////
// Pool sizes

namespace Relib {
	class CXmlDocument;
	class CXmlElement;
	class CXmlAttribute;
}

namespace rapidxml
{
	using namespace Relib;

	// Forward declarations
	class xml_document;
	
	///////////////////////////////////////////////////////////////////////
	// Parsing flags

	//! Parse flag instructing the parser to not use text of first data node as a value of parent element.
	//! Can be combined with other flags by use of | operator.
	//! Note that child data nodes of element node take precedence over its value when printing. 
	//! That is, if element has one or more child data nodes <em>and</em> a value, the value will be ignored.
	//! <br><br>
	//! See xml_document::parse() function.
	const int parse_no_element_values = 0x2;
	 
	// Compound flags
	
	//! Parse flags which represent default behavior of the parser. 
	//! This is always equal to 0, so that all other flags can be simply ored together.
	//! Normally there is no need to inconveniently disable flags by anding with their negated (~) values.
	//! This also means that meaning of each flag is a <i>negation</i> of the default setting. 
	//! For example, if flag name is rapidxml::parse_no_utf8, it means that utf-8 is <i>enabled</i> by default,
	//! and using the flag will disable it.
	//! <br><br>
	//! See xml_document::parse() function.
	const int parse_default = 0;
	
	///////////////////////////////////////////////////////////////////////
	// XML document
	
	//! This class represents root of the DOM hierarchy. 
	//! It is also an xml_node and a memory_pool through public inheritance.
	//! Use parse() function to build a DOM tree from a zero-terminated XML text string.
	//! parse() function allocates memory for nodes and attributes by using functions of xml_document, 
	//! which are inherited from memory_pool.
	//! To access root node of the document, use the document itself, as if it was an xml_node.
	//! \param char character type to use.
	class REAPI xml_document {
	public:
		//! Constructs empty XML document
		explicit xml_document( CXmlDocument& _owner, int _flags = parse_default );
		xml_document( xml_document&& other );

		~xml_document();

		// Parses zero-terminated XML string according to given flags.
		// The string must persist for the lifetime of the document.
		// In case of error, CXmlException will be thrown.
		// If you want to parse contents of a file, you must first load the file into the memory, and pass pointer to its beginning.
		// Make sure that data is zero-terminated.
		// Document can be parsed into multiple times. 
		// Each new call to parse does not clear memory pool.
		// \param text XML data to parse.
		// Returns a pointer to the root of XML tree.
		CXmlElement* parse( CUnicodeString str );

		CXmlElement* CreateCopy( const CXmlElement& other );

		// Construct an element with the given name.
		CXmlElement* AllocateElement( CUnicodeString name );
		// Construct a string.
		CUnicodeView AllocateString( CUnicodeString name );

		void Empty();

		// Accessor to an xml element creation.
		class CElementCreationKey {
		private:
			CElementCreationKey() = default;
			// Copying is prohibited.
			CElementCreationKey( CElementCreationKey& ) = delete;
			void operator=( CElementCreationKey& ) = delete;

			friend class xml_document;
		};

	private:
		CXmlDocument& owner;
		// Current document strings. Includes all the document content and additional string for added nodes.
		CArray<CUnicodeString> documentContent;
		// Parsing flags.
		int flags;
		// Pointer to the start of the text that is being parsed. Used in exceptions.
		const wchar_t* textStart;
		
		// Storage for xml elements.
		CPersistentStorage<CXmlElement, 64> elementStorage;

		///////////////////////////////////////////////////////////////////////
		// Internal character utility functions
		
		// Detect whitespace character
		// Whitespace (space \n \r \t)
		struct whitespace_pred
		{
			static bool test( wchar_t ch )
			{
				return CUnicodeString::IsCharWhiteSpace( ch );
			}
		};

		// Detect node name character
		// Node name (anything but space \n \r \t / > ? \0)
		struct node_name_pred
		{
			static const CUnicodeSet nodeNameExcludeSet;
			static unsigned char test( wchar_t ch )
			{
				return !nodeNameExcludeSet.Has( ch );
			}
		};

		// Detect attribute name character
		// Attribute name (anything but space \n \r \t / < > = ? ! \0)
		struct attribute_name_pred
		{
			static const CUnicodeSet attributeNameExcludeSet;
			static bool test( wchar_t ch )
			{
				return !attributeNameExcludeSet.Has( ch );
			}
		};

		// Detect text character (PCDATA)
		// Text (i.e. PCDATA) (anything but < \0)
		struct text_pred
		{
			static bool test( wchar_t ch )
			{
				return ch != 0 && ch != L'<';
			}
		};

		// Detect attribute value character
		// Attribute data with single quote (anything but ' \0)
		struct attribute_value_pred_single
		{
			static bool test( wchar_t ch )
			{
				return ch != 0 && ch != L'\'';
			}
		};

		struct attribute_value_pred_double
		{
			static bool test( wchar_t ch )
			{
				return ch != 0 && ch != L'\"';
			}
		};

		// Skip characters until predicate evaluates to true
		template<class StopPred>
		static void skip( const wchar_t*& text )
		{
			const wchar_t* tmp = text;
			while( StopPred::test(*tmp) ) {
				++tmp;
			}
			text = tmp;
		}

		///////////////////////////////////////////////////////////////////////
		// Internal parsing functions
		
		// Skip XML declaration (<?xml...)
		void skip_xml_declaration( const wchar_t*& text );

		// Skip XML comment (<!--...)
		void skip_comment( const wchar_t*& text );
	 
		// Skip DOCTYPE
		void skip_doctype( const wchar_t*& text );

		// Skip PI
		void skip_pi( const wchar_t*& text );
	   
		// Parse and append data
		void parse_and_append_data( CXmlElement& element, const wchar_t*& text );

		// Parse CDATA
		void skip_cdata( const wchar_t*& text );
		
		// Parse element node
		CXmlElement* parse_element( const wchar_t *&text );

		// Determine node type, and parse it
		CXmlElement* parse_node( const wchar_t*& text );

		// Parse contents of the node - children, data etc.
		void parse_node_contents( const wchar_t*& text, CXmlElement& element );

		// Parse XML attributes of the node
		void parse_node_attributes( const wchar_t*& text, CXmlElement& elem );
	};
}

