#include <XmlElement.h>
#include <StrConversions.h>
#include <ReSearch.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CXmlElement::CXmlElement( CUnicodePart _name, CXmlDocument& _document, const rapidxml::xml_document::CElementCreationKey& ) :
	name( _name ),
	document( _document ),
	parent( 0 ),
	prevSibling( 0 ),
	nextSibling( 0 ),
	firstChild( 0 ),
	lastChild( 0 )
{
}

void CXmlElement::Detach()
{
	if( IsRoot() ) {
		document.Empty();
		return;
	}
	if( parent == nullptr ) {
		return;
	}
	parent->childCount--;
	if( prevSibling == nullptr ) {
		parent->firstChild = nextSibling;
	} else {
		prevSibling->nextSibling = nextSibling;
	}
	if( nextSibling == nullptr ) {
		parent->lastChild = prevSibling;
	} else {
		nextSibling->prevSibling = prevSibling;
	}
	parent = prevSibling = nextSibling = nullptr;
}

void CXmlElement::SetText( CUnicodeString newText )
{
	text = document.createName( move( newText ) );
}

CUnicodeString CXmlElement::ToString() const
{
	return toString( 0 );
}

CXmlElement& CXmlElement::CreateChild( CUnicodePart childName )
{
	CXmlElement* newChild = document.createElement( UnicodeStr( childName ) );
	attachLastChild( newChild );
	return *newChild;
}

CXmlElement& CXmlElement::AddFirstChild( CXmlElement& child )
{
	child.Detach();
	CXmlElement* newChild = copyElementIfNecessary( child );
	assert( newChild != 0 );
	attachFirstChild( newChild );
	return *newChild;
}

CXmlElement& CXmlElement::AddLastChild( CXmlElement& child )
{
	child.Detach();
	CXmlElement* newChild = copyElementIfNecessary( child );
	assert( newChild != 0 );
	attachLastChild( newChild );
	return *newChild;
}

CXmlElement& CXmlElement::AddPrev( CXmlElement& sibling )
{
	sibling.Detach();
	CXmlElement* newSibling = copyElementIfNecessary( sibling );
	assert( newSibling != 0 );
	attachPrevSibling( newSibling );
	return *newSibling;
}

CXmlElement& CXmlElement::AddNext( CXmlElement& sibling )
{
	sibling.Detach();
	CXmlElement* newSibling = copyElementIfNecessary( sibling );
	assert( newSibling != 0 );
	attachNextSibling( newSibling );
	return *newSibling;
}

void CXmlElement::DetachAllChildren()
{
	for( CXmlElement* child = FirstChild(); child != 0; child = child->Next() ) {
		child->prevSibling = child->nextSibling = child->parent = 0;
	}
	FastDetachAllChildren();
}

void CXmlElement::FastDetachAllChildren()
{
	lastChild = firstChild = nullptr;
	childCount = 0;
}

bool CXmlElement::HasAttribute( CUnicodePart attrName ) const
{
	return Has( attributes, attrName, EqualByAction( &CXmlAttribute::Name ) );
}

void CXmlElement::AddAttribute( CUnicodePart attrName, CUnicodePart value )
{
	assert( !HasAttribute( attrName ) );
	CUnicodeView nameStr = document.createName( UnicodeStr( attrName ) );
	CUnicodeView valueStr = document.createName( UnicodeStr( value ) );
	attributes.Add( nameStr, valueStr );
}

CUnicodePart CXmlElement::GetAttributeValueText( CUnicodePart attrName ) const
{
	for( const auto& attr : attributes ) {
		if( attr.Name() == attrName ) {
			return attr.GetValueText();
		}
	}
	assert( false );
	return CUnicodePart();
}

void CXmlElement::SetAttributeValueText( CUnicodePart attrName, CUnicodeString value )
{
	CUnicodeView valueStr = document.createName( move( value ) );

	for( auto& attr : attributes ) {
		if( attr.Name() == attrName ) {
			attr.setValueText( valueStr );
			return;
		}
	}
	
	CUnicodeView nameStr = document.createName( UnicodeStr( attrName ) );
	attributes.Add( nameStr, valueStr );
}

void CXmlElement::DeleteAttribute( CUnicodePart attrName )
{
	for( int i = 0; i < attributes.Size(); i++ ) {
		if( attributes[i].Name() == attrName ) {
			attributes.DeleteAt( i );
			return;
		}
	}
	assert( false );
}

// Check if the element is allocated on another document and create a local copy if that's the case.
// Return the copy if it's created or the element itself.
CXmlElement* CXmlElement::copyElementIfNecessary( CXmlElement& element ) const
{
	CXmlElement* newElem;
	if( &element.document == &document ) {
		newElem = &element;
	} else {
		newElem = document.copyElement( element );
	}
	return newElem;
}

// Child attachment with no additional checks.
// Child must have no parent.
void CXmlElement::attachFirstChild( CXmlElement* child )
{
	assert( child != 0 && child->parent == 0 );
	if( firstChild != 0 ) {
		// Insert child in the siblings list.
		firstChild->prevSibling = child;
		child->nextSibling = firstChild;
	} else {
		// Element has no children, we should assign both last and first child.
		assert( lastChild == 0 );
		lastChild = child;
	}
	firstChild = child;
	child->parent = this;
	childCount++;
}

void CXmlElement::attachLastChild( CXmlElement* child )
{
	assert( child != 0 && child->parent == 0 );
	if( lastChild != 0 ) {
		// Insert child in the siblings list.
		lastChild->nextSibling = child;
		child->prevSibling = lastChild;
	} else {
		// Element has no children, we should assign both last and first child.
		assert( firstChild == 0 );
		firstChild = child;
	}
	lastChild = child;
	child->parent = this;
	childCount++;
}

void CXmlElement::attachPrevSibling( CXmlElement* sibling )
{
	assert( parent != 0 );
	assert( sibling != 0 && sibling->parent == 0 );
	if( prevSibling == 0 ) {
		// Sibling is the new first child.
		assert( parent->firstChild == this );
		parent->firstChild = sibling;
	} else {
		prevSibling->nextSibling = sibling;
	}
	prevSibling = sibling;
	sibling->nextSibling = this;
	sibling->parent = parent;
	parent->childCount++;
}

void CXmlElement::attachNextSibling( CXmlElement* sibling )
{
	assert( parent != 0 );
	assert( sibling != 0 && sibling->parent == 0 );
	if( nextSibling == 0 ) {
		// Sibling is the new last child.
		assert( parent->lastChild == this );
		parent->lastChild = sibling;
	} else {
		nextSibling->prevSibling = sibling;
	}
	nextSibling = sibling;
	sibling->prevSibling = this;
	sibling->parent = parent;
	parent->childCount++;
}

static const wchar_t xmlOpenBracket = L'<';
static const wchar_t xmlCloseBracket = L'>';
static const wchar_t xmlCloseTag = L'/';
static const wchar_t xmlAttributeWrapper = L'\"';
static const wchar_t xmlIndentSymbol = L'\t';
static const wchar_t xmlNewline[] = L"\r\n";
// Convert to string. New lines are indented by a given amount.
CUnicodeString CXmlElement::toString( int indent ) const
{
	CUnicodeString result;
	for( int i = 0; i < indent; i++ ) {
		result += xmlIndentSymbol;
	}
	result += xmlOpenBracket;
	result += Name();
	for( const auto& attr : attributes ) {
		result += L' ';
		result += attr.Name();
		result += L'=';
		result += xmlAttributeWrapper;
		result += attr.GetValueText();
		result += xmlAttributeWrapper;
	}

	if( firstChild == 0 && text.IsEmpty()) {
		result += xmlCloseTag;
		result += xmlCloseBracket;
	} else if( firstChild == 0 && !text.IsEmpty() ) {
		result += xmlCloseBracket;
		result += GetText();
		appendClosedNameTag( result, 0 );
	} else {
		result += xmlCloseBracket;
		result += L' ';
		result += GetText();
		result += xmlNewline;
		for( const CXmlElement* child = FirstChild(); child != 0; child = child->Next() ) {
			result += child->toString( indent + 1 );
			result += xmlNewline;
		}
		appendClosedNameTag( result, indent );
	}
	return result;
}

void CXmlElement::appendClosedNameTag( CUnicodeString& result, int indent ) const
{
	for( int i = 0; i < indent; i++ ) {
		result += xmlIndentSymbol;
	}
	result += xmlOpenBracket;
	result += xmlCloseTag;
	result += Name();
	result += xmlCloseBracket;
}

CXmlElementAccessor CXmlElement::Siblings()
{
	return parent == 0 ? CXmlElementAccessor( this ) : CXmlElementAccessor( parent->firstChild );
}

CXmlConstElementAccessor CXmlElement::Siblings() const
{
	return parent == 0 ? CXmlConstElementAccessor( this ) : CXmlConstElementAccessor( parent->firstChild );
}

CXmlElementAccessor CXmlElement::Children()
{
	return CXmlElementAccessor( firstChild );
}

CXmlConstElementAccessor CXmlElement::Children() const
{
	return CXmlConstElementAccessor( firstChild );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

