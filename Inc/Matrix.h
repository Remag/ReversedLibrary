#pragma once

#include <TemplateUtils.h>
#include <Redefs.h>

namespace Relib {

// Order of matrix elements in the array.
enum TMatrixOrder {
	MO_RowMajor,
	MO_ColumnMajor
};

//////////////////////////////////////////////////////////////////////////

// Matrix of fundamental types.
template <class Type, int dimX, int dimY, TMatrixOrder order = MO_ColumnMajor>
class CMatrix {
public:
	staticAssert( Types::IsFundamental<Type>::Result );

	typedef Type ElemType;

	// Creates a zero matrix.
	CMatrix();
	// Creates a diagonal matrix with diagElem.
	explicit CMatrix( Type diagElem );
	// Create a matrix without field initialization.
	static CMatrix CreateRawMatrix();

	static int SizeX()
		{ return dimX; }
	static int SizeY()
		{ return dimY; }
	static TMatrixOrder MatrixOrder()
		{ return order; }

	// Element access.
	Type& operator()( int xPos, int yPos );
	Type operator()( int xPos, int yPos ) const;
	void GetRow( int rowPos, CVector<Type, dimX>& row ) const;
	void GetColumn( int columnPos, CVector<Type, dimY>& column ) const;

	// Set methods for rows/columns.
	void SetRow( int rowPos, const CVector<Type, dimX>& row );
	void SetColumn( int columnPos, const CVector<Type,dimY>& column );

	// Direct buffer access.
	Type* Ptr()
		{ return matrixData; }
	const Type* Ptr() const
		{ return matrixData; }

	// Comparison.
	bool operator==( const CMatrix& other ) const;
	bool operator!=( const CMatrix& other ) const
		{ return !operator==( other ); }
	bool IsNull() const;
	bool IsIdentity() const;

	void Transpose();
	CMatrix<Type, dimY, dimX> GetTransposedMatrix() const;

	int HashKey() const;

private:
	Type matrixData[dimX * dimY];

	typedef typename Types::Conditional<order == MO_RowMajor, Types::TrueType, Types::FalseType>::Result TRowMajorMarker;

	Type getElem( int x, int y, Types::TrueType rowMarker ) const;
	Type& getElem( int x, int y, Types::TrueType rowMarker );
	Type getElem( int x, int y, Types::FalseType columnMarker ) const;
	Type& getElem( int x, int y, Types::FalseType columnMarker );

	void getRow( int rowPos, CVector<Type, dimX>& row, Types::TrueType rowMarker ) const;
	void getColumn( int columnPos, CVector<Type, dimY>& column, Types::TrueType rowMarker ) const;
	void getRow( int rowPos, CVector<Type, dimX>& row, Types::FalseType columnMarker ) const;
	void getColumn( int columnPos, CVector<Type, dimY>& column, Types::FalseType columnMarker ) const;

	void setRow( int rowPos, const CVector<Type, dimX>& row, Types::TrueType rowMarker );
	void setColumn( int columnPos, const CVector<Type,dimY>& column, Types::TrueType rowMarker );
	void setRow( int rowPos, const CVector<Type, dimX>& row, Types::FalseType columnMarker );
	void setColumn( int columnPos, const CVector<Type,dimY>& column, Types::FalseType columnMarker );

	// Constructor for raw matrix creation.
	class CRawCreationTag {};
	explicit CMatrix( CRawCreationTag ) {}
};

//////////////////////////////////////////////////////////////////////////

template<class Type, int dimX, int dimY, TMatrixOrder order>
CMatrix<Type, dimX, dimY, order>::CMatrix()
{
	memset( matrixData, 0, dimX * dimY * sizeof( Type ) );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
CMatrix<Type, dimX, dimY, order>::CMatrix( Type diagElem )
{
	staticAssert( dimX == dimY );
	memset( matrixData, 0, dimX * dimY * sizeof( Type ) );
	for( int i = 0; i < dimX; i++ ) {
		matrixData[i * dimX + i] = diagElem;
	}
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
CMatrix<Type, dimX, dimY, order> CMatrix<Type, dimX, dimY, order>::CreateRawMatrix()
{
	CRawCreationTag tag;
	CMatrix<Type, dimX, dimY, order> rawMatrix( tag );
	return rawMatrix;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
Type CMatrix<Type, dimX, dimY, order>::operator()( int xPos, int yPos ) const
{
	assert( xPos >= 0 && xPos < dimX );
	assert( yPos >= 0 && yPos < dimY );
	return getElem( xPos, yPos, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
Type& CMatrix<Type, dimX, dimY, order>::operator()( int xPos, int yPos )
{
	assert( xPos >= 0 && xPos < dimX );
	assert( yPos >= 0 && yPos < dimY );
	return getElem( xPos, yPos, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
void CMatrix<Type, dimX, dimY, order>::GetRow( int rowPos, CVector<Type, dimX>& row ) const
{
	getRow( rowPos, row, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
void CMatrix<Type, dimX, dimY, order>::GetColumn( int columnPos, CVector<Type,dimY>& column ) const
{
	getColumn( columnPos, column, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
void CMatrix<Type, dimX, dimY, order>::SetRow( int rowPos, const CVector<Type, dimX>& row )
{
	setRow( rowPos, row, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
void CMatrix<Type, dimX, dimY, order>::SetColumn( int columnPos, const CVector<Type, dimY>& column )
{
	setColumn( columnPos, column, TRowMajorMarker() );
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
bool CMatrix<Type, dimX, dimY, order>::operator==( const CMatrix<Type, dimX, dimY, order>& other ) const
{
	const Type* ptr = matrixData;
	const Type* otherPtr = other.matrixData;
	for( int i = 0; i < dimX * dimY; i++, ptr++, otherPtr++ ) {
		if( *otherPtr != *ptr ) {
			return false;
		}
	}
	return true;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
bool CMatrix<Type, dimX, dimY, order>::IsNull() const
{
	for( const auto& elem : matrixData ) {
		if( elem != 0 ) {
			return false;
		}
	}
	return true;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
bool CMatrix<Type, dimX, dimY, order>::IsIdentity() const
{
	staticAssert( dimX == dimY );
	for( int i = 0; i < dimX; i++ ) {
		for( int j = 0; j < dimY; j++ ) {
			if( ( i == j && matrixData[i * dimX + j] != static_cast<Type>( 1 ) ) || 
				( i != j && matrixData[i* dimX + j] != 0 ) )
			{
				return false;
			}
		}
	}
	return true;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
void CMatrix<Type, dimX, dimY, order>::Transpose()
{
	staticAssert( dimX == dimY );
	for( int i = 0; i < dimX; i++ ) {
		for( int j = i + 1; j < dimY; j++ ) {
			swap( matrixData[i * dimX + j], matrixData[j * dimX + i] );
		}
	}
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
CMatrix<Type, dimY, dimX> CMatrix<Type, dimX, dimY, order>::GetTransposedMatrix() const
{
	CMatrix<Type, dimY, dimX> result = CMatrix<Type, dimY, dimX>::CreateRawMatrix();
	for( int i = 0; i < dimY; i++ ) {
		for( int j = 0; j < dimX; j++ ) {
			result( i, j ) = operator()( j, i );
		}
	}
	return result;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
int CMatrix<Type, dimX, dimY, order>::HashKey() const
{
	int resultHash = 0;

	for( const auto& elem : matrixData ) {
		AddToHashKey( CDefaultHash<Type>::HashKey( elem, resultHash ) );
	}
	return resultHash;
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
Type CMatrix<Type, dimX, dimY, order>::getElem( int x, int y, Types::TrueType rowMarker ) const
{
	return matrixData[y * dimX + x];
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
Type& CMatrix<Type, dimX, dimY, order>::getElem( int x, int y, Types::TrueType rowMarker )
{
	return matrixData[y * dimX + x];
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
Type CMatrix<Type, dimX, dimY, order>::getElem( int x, int y, Types::FalseType ) const
{
	return matrixData[x * dimY + y];
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
Type& CMatrix<Type, dimX, dimY, order>::getElem( int x, int y, Types::FalseType )
{
	return matrixData[x * dimY + y];
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::getRow( int rowPos, CVector<Type, dimX>& row, Types::TrueType rowMarker ) const
{
	memcpy( row.Ptr(), matrixData + rowPos * dimX, dimX * sizeof( Type ) );
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::getColumn( int columnPos, CVector<Type, dimY>& column, Types::FalseType columnMarker ) const
{
	memcpy( column.Ptr(), matrixData + columnPos * dimY, dimY * sizeof( Type ) );
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::getRow( int rowPos, CVector<Type, dimX>& row, Types::FalseType columnMarker ) const
{
	assert( rowPos >= 0 && rowPos < dimY );
	for( int i = 0; i < dimX; i++ ) {
		row[i] = getElem( i, rowPos, TRowMajorMarker() );
	}
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::getColumn( int columnPos, CVector<Type, dimY>& column, Types::TrueType rowMarker ) const
{
	assert( columnPos >= 0 && columnPos < dimX );
	for( int i = 0; i < dimY; i++ ) {
		column[i] = getElem( columnPos, i, TRowMajorMarker() );
	}
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::setRow( int rowPos, const CVector<Type, dimX>& row, Types::TrueType rowMarker )
{
	memcpy( matrixData + rowPos * dimX, row.Ptr(), dimX * sizeof( Type ) );
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::setColumn( int columnPos, const CVector<Type, dimY>& column, Types::FalseType columnMarker )
{
	memcpy( matrixData + columnPos * dimY, column.Ptr(), dimY * sizeof( Type ) );
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::setRow( int rowPos, const CVector<Type, dimX>& row, Types::FalseType columnMarker )
{
	assert( rowPos >= 0 && rowPos < dimY );
	for( int i = 0; i < dimX; i++ ) {
		getElem( i, rowPos, TRowMajorMarker() ) = row[i];
	}
}

template<class Type, int dimX, int dimY, TMatrixOrder order /*= MO_ColumnMajor*/>
void CMatrix<Type, dimX, dimY, order>::setColumn( int columnPos, const CVector<Type, dimY>& column, Types::TrueType rowMarker )
{
	assert( columnPos >= 0 && columnPos < dimX );
	for( int i = 0; i < dimY; i++ ) {
		getElem( columnPos, i, TRowMajorMarker() ) = column[i];
	}
}

//////////////////////////////////////////////////////////////////////////

template<class Type, int dimM, int dimN, int dimP, TMatrixOrder order>
CMatrix<Type, dimP, dimN, order> operator*( const CMatrix<Type, dimM, dimN, order>& left, const CMatrix<Type, dimP, dimM, order>& right )
{
	CMatrix<Type, dimP, dimN, order> result;
	for( int p = 0; p < dimP; p++ ) {
		for( int n = 0; n < dimN; n++ ) {
			for( int m = 0; m < dimM; m++  ) {
				result( p, n ) += left( m, n ) * right( p, m );
			}
		}
	}
	return result;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
CVector<Type, dimY> operator*( const CMatrix<Type, dimX, dimY, order>& matrix, const CVector<Type, dimX>& vec )
{
	CVector<Type, dimY> result;
	for( int y = 0; y < dimY; y++ ) {
		for( int x = 0; x < dimX; x++ ) {
			result[y] += matrix( x, y ) * vec[x];
		}
	}
	return result;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
CVector<Type, dimX> operator*( const CVector<Type, dimY>& vec, const CMatrix<Type, dimX, dimY, order>& matrix )
{
	CVector<Type, dimX> result;
	for( int x = 0; x < dimX; x++ ) {
		for( int y = 0; y < dimY; y++ ) {
			result[x] += matrix( x, y ) * vec[y];
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

template <class MatType>
using CMatrix2 = CMatrix<MatType, 2, 2>;

template <class MatType>
using CMatrix3 = CMatrix<MatType, 3, 3>;

template <class MatType>
using CMatrix4 = CMatrix<MatType, 4, 4>;

//////////////////////////////////////////////////////////////////////////

// Serialization.

template<class Type, int dimX, int dimY, TMatrixOrder order>
inline CArchiveWriter& operator<<( CArchiveWriter& archive, const CMatrix<Type, dimX, dimY, order>& matrix )
{
	for( int i = 0; i < dimX; i++ ) {
		for( int j = 0; j < dimY; j++ ) {
			archive << matrix( i, j );
		}
	}
	return archive;
}

template<class Type, int dimX, int dimY, TMatrixOrder order>
inline CArchiveReader& operator>>( CArchiveReader& archive, CMatrix<Type, dimX, dimY, order>& matrix )
{
	for( int i = 0; i < dimX; i++ ) {
		for( int j = 0; j < dimY; j++ ) {
			archive >> matrix( i, j );
		}
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////

namespace Types {

// "Is Matrix" support class.
// This class has a true type if A is a CMatrix.
template<class A>
struct IsMatrix : public FalseType {
};

template<class Type, int dimX, int dimY, TMatrixOrder matOrder>
struct IsMatrix< CMatrix<Type, dimX, dimY, matOrder> > : public TrueType {
};

}	// namespace Types.

}	// namespace Relib.

