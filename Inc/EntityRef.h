#pragma once
#include <Reassert.h>
#include <Entity.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

struct CFullEntityData;
class CEntity;
class CEntityConstRef;
bool operator==( CEntityConstRef left, CEntityConstRef right );

namespace RelibInternal {

// Common reference constant operations.
class REAPI CEntityRefConstData {
public:
	CEntityRefConstData() = default;
	explicit CEntityRefConstData( const CFullEntityData* _refData, int _generation ) : 
		refData( const_cast<CFullEntityData*>( _refData ) ), generation( _generation ) {}

	// Check if the reference has never been initialized.
	bool IsNull() const
		{ return refData == nullptr; }
	// Check if it's safe to dereference the value.
	bool IsValid() const
		{ return !isExpired(); }

	// Dereference operations.
	const CEntity& GetEntity() const
		{ assert( IsValid() ); return refData->Entity; }

	const CEntity* operator->() const
		{ return &GetEntity(); }

	// Comparison operator needs access to raw data.
	friend bool Relib::operator==( CEntityConstRef left, CEntityConstRef right );

protected:
	// Check if the referenced entity has been deleted.
	bool isExpired() const;

	int getGeneration() const
		{ return generation; }
	CFullEntityData* getRefData() const
		{ return refData; }
	
private:
	// Full data is stored as a non-const pointer because constant data is used as a base class for non-const data.
	CFullEntityData* refData = nullptr;
	int generation = 0;
};

//////////////////////////////////////////////////////////////////////////

// Common reference operations.
class REAPI CEntityRefData : public CEntityRefConstData {
public:
	CEntityRefData() = default;
	explicit CEntityRefData( CFullEntityData* _refData, int _generation ) : CEntityRefConstData( _refData, _generation ) {}

	// Dereference operations.
	using CEntityRefConstData::GetEntity;
	CEntity& GetEntity()
		{ return const_cast<CEntity&>( CEntityRefConstData::GetEntity() ); }

	using CEntityRefConstData::operator->;
	CEntity* operator->()
		{ return &GetEntity(); }
};

//////////////////////////////////////////////////////////////////////////

inline bool CEntityRefConstData::isExpired() const
{
	return IsNull() || generation != refData->Generation;
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Reference to an entity with constant-only access. Allows fast access to its storage data.
class CEntityConstRef : public RelibInternal::CEntityRefConstData {
public:
	using CEntityRefConstData::CEntityRefConstData;
};

//////////////////////////////////////////////////////////////////////////

// Reference to an entity with constant-only access. Allows fast access to its storage data.
class CEntityRef : public RelibInternal::CEntityRefData {
public:
	using CEntityRefData::CEntityRefData;

	operator CEntityConstRef() const
		{ return CEntityConstRef( getRefData(), getGeneration() ); }
};

//////////////////////////////////////////////////////////////////////////

// Comparison.
inline bool operator==( CEntityConstRef left, CEntityConstRef right )
{
	const RelibInternal::CEntityRefConstData leftData = left;
	const RelibInternal::CEntityRefConstData rightData = right;
	return leftData.refData == rightData.refData && leftData.generation == rightData.generation;
}

inline bool operator!=( CEntityConstRef left, CEntityConstRef right )
{
	return !( left == right );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

