#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Shape consisting of multiple convex shapes.
// This shape is not convex, which is not important for intersection tests but is important for containment tests.
// If the containment test is required, a restriction applies:
// The sub shapes must not intersect. Shapes with a common edge are treated as intersecting.
template <class T>
class CComplexShape : public CShape<T> {
public:
	typedef T Type;

	CComplexShape() = default;

	const CArray<CPtrOwner<CShape<T>>>& Shapes() const
		{ return shapes; }

	void SetShapes( CArray<CPtrOwner<CShape<T>>> newShapes )
		{ shapes = move( newShapes ); }
	void AddShape( CPtrOwner<CShape<T>> newShape )
		{ shapes.Add( move( newShape ) ); }

	// CConvexShape.
	virtual THitboxShapeType GetType() const override final
		{ return HST_Complex; }
	virtual CAARect<T> GetBoundRect( CMatrix3<float> transformation ) const override final;

private:
	// Shapes in the union.
	CArray<CPtrOwner<CShape<T>>> shapes;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
CAARect<T> CComplexShape<T>::GetBoundRect( CMatrix3<float> transformation ) const
{
	CAARect<T> result;
	for( const auto& shape : shapes ) {
		result = GetRectUnion( result, shape->GetBoundRect( transformation ) );
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

