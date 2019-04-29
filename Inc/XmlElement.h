#pragma once
#include <BaseString.h>
#include <Array.h>
#include <XmlDocument.h>

namespace Relib {

class CXmlConstElementAccessor;
class CXmlElementAccessor;
//////////////////////////////////////////////////////////////////////////

// An XML attribute.
class REAPI CXmlAttribute {
public:
	CXmlAttribute( CUnicodePart _name, CUnicodePart _value ) : name( _name ), valueStr( move( _value ) ) {}

	CUnicodePart Name() const
		{ return name; }

	// Getting/Setting attribute value in a string format.
	CUnicodePart GetValueText() const
		{ return valueStr; }
	// Receive the value of the attribute and try to convert it with the value function.
	// If value fails to be converted, defaultValue is returned.
	template <class Type>
	Type GetValue( const Type& defaultValue ) const;

	// Elements can change an attribute value.
	friend class CXmlElement;

private:
	const CUnicodePart name;
	CUnicodePart valueStr;

	void setValueText( CUnicodePart docText )
		{ valueStr = docText; }
};

template <class Type>
inline Type CXmlAttribute::GetValue( const Type& defaultValue ) const
{
	auto resultValue = Value<Type>( valueStr );
	return resultValue.IsValid() ? move( *resultValue ) : copy( defaultValue );
}

//////////////////////////////////////////////////////////////////////////

// An XML element.
class REAPI CXmlElement {
public:
	// Nodes shouldn't be created outside of an XML document.
	CXmlElement( CUnicodePart name, CXmlDocument& document, const rapidxml::xml_document::CElementCreationKey& );

	CXmlDocument& GetDocument() const
		{ return document; }
	bool IsRoot() const
		{ return &document.GetRoot() == this; }

	CUnicodePart Name() const
		{ return name; }

	// Detach the element from its parent.
	// Element is not deleted until the whole document is cleared.
	void Detach();

	// Get the text of the element. Text of the first data node is used.
	CUnicodePart GetText() const
		{ return text; }
	void SetText( CUnicodeString newText );
	// Convert the subtree to string.
	CUnicodeString ToString() const;

	// Accessing siblings and children.
	int GetChildrenCount() const
		{ return childCount; }
	const CXmlElement* Prev() const
		{ return prevSibling; }
	const CXmlElement* Next() const
		{ return nextSibling; }
	const CXmlElement* GetParent() const
		{ return parent; }
	const CXmlElement* FirstChild() const
		{ return firstChild; }
	const CXmlElement* LastChild() const
		{ return lastChild; }

	CXmlElement* Prev()
		{ return prevSibling; }
	CXmlElement* Next()
		{ return nextSibling; }
	CXmlElement* GetParent()
		{ return parent; }
	CXmlElement* FirstChild()
		{ return firstChild; }
	CXmlElement* LastChild()
		{ return lastChild; }
	
	// Create a child element and insert in at the back of the element's children list.
	CXmlElement& CreateChild( CUnicodePart name );
	
	// Insertion of elements.
	// Element is detached from its previous position.
	// If element is not from the parent's document, it is copied there.
	// Insertion of children.
	CXmlElement& AddFirstChild( CXmlElement& child );
	CXmlElement& AddLastChild( CXmlElement& child );
	// Insertion of siblings.
	CXmlElement& AddPrev( CXmlElement& sibling );
	CXmlElement& AddNext( CXmlElement& sibling );
	// Remove all children.
	void DetachAllChildren();
	// Fast detach. Children are left in an invalid state and won't know that their parent disowned them.
	void FastDetachAllChildren();

	// Methods for reading and editing the attributes
	int GetAttributesCount() const
		{ return attributes.Size(); }
	bool HasAttribute( CUnicodePart name ) const;

	// Add an attribute with a given name/value. Attribute with this name must not be present in the element.
	void AddAttribute( CUnicodePart name, CUnicodePart value );
	template <typename Type>
	void AddAttribute( CUnicodePart name, const Type& value );
	// Get a string value for the attribute. The attribute must be present.
	CUnicodePart GetAttributeValueText( CUnicodePart name ) const;
	// Get a string value for the attribute.
	// If the attribute is not present, a new one is added with the given name.
	void SetAttributeValueText( CUnicodePart name, CUnicodeString value );

	// Get the value of the attribute and try to convert it with the Value function.
	// If value fails to be converted, defaultValue is returned.
	template <typename Type>
	auto GetAttributeValue( CUnicodePart name, const Type& defaultValue ) const; 
	// Sets the attribute value. Value is converted to string with the UnicodeStr function.
	template <typename Type>
	void SetAttributeValue( CUnicodePart name, const Type& value );

	// Deletes an attribute with the given name. Attribute must exist.
	void DeleteAttribute( CUnicodePart name );

	// RapidXML classes need access to fast children attachment.
	friend class rapidxml::xml_document;

	// Range based for loops support.
	
	// These methods return an internal class which is not useful outside of for loops.
	// Intended use-case:
	// for( const auto& child : element->Children() ) {
	//	<<child iterates through element's children>>
	//	}
	CXmlElementAccessor Children();
	CXmlConstElementAccessor Children() const;

	// Methods for siblings iteration. Current element is also iterated through.
	CXmlElementAccessor Siblings();
	CXmlConstElementAccessor Siblings() const;
	
	// Attribute iteration.
	// This method returns a reference to an array and can actually be useful outside for loops.
	// It is still recommended to use attribute access methods from the element.
	CArrayView<CXmlAttribute> Attributes() const
		{ return attributes; }
	CArrayBuffer<CXmlAttribute> Attributes()
		{ return attributes; }

private:
	CUnicodePart text;
	const CUnicodePart name;

	// Document that owns the element.
	CXmlDocument& document;
	CXmlElement* parent = nullptr;

	// Siblings.
	CXmlElement* prevSibling = nullptr;
	CXmlElement* nextSibling = nullptr;
	// Children.
	CXmlElement* firstChild = nullptr;
	CXmlElement* lastChild = nullptr;
	int childCount = 0;

	// Attributes.
	CArray<CXmlAttribute> attributes;
	
	CXmlElement* copyElementIfNecessary( CXmlElement& element ) const;
	void attachFirstChild( CXmlElement* child );
	void attachLastChild( CXmlElement* child );
	void attachPrevSibling( CXmlElement* sibling );
	void attachNextSibling( CXmlElement* sibling );
	CUnicodeString toString( int indent ) const;
	void appendClosedNameTag( CUnicodeString& result, int indent ) const;

	template <typename T>
	static const T& downcastStringType( const T& type, Types::FalseType relibStrMarker );
	template <typename T>
	static CUnicodePart downcastStringType( const T& type, Types::TrueType relibStrMarker );
};

//////////////////////////////////////////////////////////////////////////

template <typename Type>
void CXmlElement::AddAttribute( CUnicodePart attrName, const Type& value )
{
	const auto valueStr = UnicodeStr( value );
	AddAttribute( attrName, CUnicodePart( valueStr ) );
}

template <typename Type>
auto CXmlElement::GetAttributeValue( CUnicodePart attrName, const Type& defaultValue ) const 
{
	// Downcast all string types to string part.
	const auto& value = downcastStringType( defaultValue, Types::IsRelibString<Type, wchar_t>() );
	for( const auto& attr : attributes ) {
		if( attr.Name() == attrName ) {
			return attr.GetValue( value );
		}
	}
	return copy( value );
}

template <typename T>
const T& CXmlElement::downcastStringType( const T& type, Types::FalseType )
{
	return type;
}

template <typename T>
CUnicodePart CXmlElement::downcastStringType( const T& type, Types::TrueType )
{
	return type;
}

template <typename Type>
void CXmlElement::SetAttributeValue( CUnicodePart attrName, const Type& value )
{
	SetAttributeValueText( attrName, UnicodeStr( value ) );
}

//////////////////////////////////////////////////////////////////////////

// Range-base for loop support.

// Xml element accessor. Similar to iterators but begin and end methods actually has accessors as arguments
// Supports bare minimum of methods needed for range-based for loops. Any other usage is discouraged.
class CXmlElementAccessor {
public:
	explicit CXmlElementAccessor( CXmlElement* elem ) : curElement( elem ) {}

	CXmlElement& operator*()
		{ return *curElement; }
	// An increment operator.
	// Range-based for loops don't require the ++operator to return a value.
	void operator++()
		{ curElement = curElement->Next(); }
	bool operator!=( const CXmlElementAccessor& ) const
		{ return curElement != nullptr; }

private:
	CXmlElement* curElement;
};

class CXmlConstElementAccessor {
public:
	explicit CXmlConstElementAccessor( const CXmlElement* elem ) : curElement( elem ) {}

	const CXmlElement& operator*()
	{ return *curElement; }
	// An increment operator.
	// Range-based for loops don't require the ++operator to return a value.
	void operator++()
		{ curElement = curElement->Next(); }
	bool operator!=( const CXmlConstElementAccessor& ) const
		{ return curElement != nullptr; }

private:
	const CXmlElement* curElement;
};

inline CXmlElementAccessor begin( const CXmlElementAccessor& accessor )
{
	return accessor;
}

inline CXmlElementAccessor end( const CXmlElementAccessor& )
{
	return CXmlElementAccessor( nullptr );
}

inline CXmlConstElementAccessor begin( const CXmlConstElementAccessor& accessor )
{
	return accessor;
}

inline CXmlConstElementAccessor end( const CXmlConstElementAccessor& )
{
	return CXmlConstElementAccessor( 0 );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
