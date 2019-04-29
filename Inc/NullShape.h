#pragma once
#include <Shape.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Null hitbox. Returns false for all collisions.
template <class T>
class CNullShape : public CShape<T> {
public:
	virtual THitboxShapeType GetType() const override final
		{ return HST_Null; }
	virtual CAARect<T> GetBoundRect( const CMatrix3<T>& ) const override final
		{ return CAARect<T>{}; }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

