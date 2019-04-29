// Sorting algortithms.

namespace Relib {

namespace Sort {

namespace Internal {

//////////////////////////////////////////////////////////////////////////

// A quick sort step. Take the mean element from the array and rearrange the elements according to their relation to the mean.
// Bigger elements go to the right part of the array. Smaller ones go to the left.
// Returns a mean index.
template <class LessAction, class Type>
inline int divideArray( Type* arr, int arraySize, const LessAction& less )
{
	assert( arraySize >= 2 );
	// Put the mean at the start.
	const int meanPosition = 0;
	swap( arr[meanPosition], arr[arraySize / 2] );
	int currentLessPos = 0;
	int currentGreaterPos = arraySize;
	for(;;) {
		// Find one element that is incorrectly placed and is less than mean.
		do {
			currentLessPos++;
		} while( currentLessPos < arraySize && less( arr[currentLessPos], arr[meanPosition] ) );
		// Find one element that is incorrectly placed and is greater than mean.
		do {
			currentGreaterPos--;
		} while( currentGreaterPos > 0 && less( arr[meanPosition], arr[currentGreaterPos] ) );
		
		if( currentGreaterPos < currentLessPos ) {
			// All elements are actually placed correctly, the rearrangement is done.
			break;
		}
		// Swap the incorrectly placed elements.
		swap( arr[currentLessPos], arr[currentGreaterPos] );
	}
	// Place the mean into a correct position.
	if( meanPosition != currentGreaterPos ) {
		swap( arr[meanPosition], arr[currentGreaterPos] );
	}
	// CurrentGreaterPos now contains the mean.
	return currentGreaterPos;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Internal.

//////////////////////////////////////////////////////////////////////////

// Insertion sort with a comparison class.
template <class LessAction, class Type>
void InSort( Type* arr, int elemCount, const LessAction& less )
{
	assert( elemCount >= 0 );
	for( int i = elemCount - 1; i > 0; i-- ) {
		// Find an element that is not in order.
		const int insertIndex = i - 1;
		if( less( arr[i], arr[insertIndex] ) ) {
			// Find an insertion point for the element.
			int insPos = elemCount;
			for( int j = i + 1; j < elemCount; j++ ) {
				if( less( arr[insertIndex], arr[j] ) ) {
					insPos = j;
					break;
				}
			}
			// Insert the element in its place.
			Type temp = move( arr[insertIndex] );
			ArrayInternalMove( arr + i, arr + insertIndex, insPos - i );
			arr[insPos - 1] = move( temp );
		}
	}
}

// Selection sort with a comparison class.
template <class LessAction, class Type>
void SelSort( Type* arr, int elemCount, const LessAction& less )
{
	assert( elemCount >= 0 );
	for( int i = elemCount - 1; i > 0; i-- ) {
		int biggest = i;
		for( int j = i - 1; j >= 0; j-- ) {
			if( less( arr[biggest], arr[j] ) ) {
				biggest = j;
			}
		}
		if( i != biggest ) {
			swap( arr[biggest], arr[i] );
		}
	}
}

static const int addressSpaceBitCount = CHAR_BIT * sizeof( void* );
// Arrays with this or smaller size will be sorted with SelSort.
static const int quickSortCutoff = 8;

// Quick sort with an arbitrary comparison function.
template <class LessAction, class Type>
void QSort( Type* arr, int elemCount, const LessAction& less )
{
	assert( elemCount >= 0 );
	if( elemCount <= 1 ) {
		return;
	}
	// We are going to divide an array into two parts and put the bigger one on the stack to evaluate it later.
	// In the worst case scenario we have to divide array in half each time.
	// The biggest possible array takes all the address space.
	// That's how the maximum possible stack size is calculated.
	Type* arrayStack[addressSpaceBitCount];
	int sizeStack[addressSpaceBitCount];
	int stackPos = 0;
	Type* subArray = arr;
	int subArraySize = elemCount;

	for( ;; ) {
		if( subArraySize <= quickSortCutoff ) {
			// Insertion sort is faster at this point.
			Sort::SelSort( subArray, subArraySize, less );
		} else {
			const int mean = Internal::divideArray( subArray, subArraySize, less );
			const int rightHalfSize = subArraySize - mean - 1;
			if( mean < rightHalfSize ) {
				// The right half is bigger.
				if( rightHalfSize >= 2 ) {
					// Put the right half in the stack for further evaluation.
					arrayStack[stackPos] = subArray + mean + 1;
					sizeStack[stackPos] = rightHalfSize;
					stackPos++;
				}
				if( mean >= 2 ) {
					// Rerun the algorithm for the smaller part.
					subArraySize = mean;
					continue;
				}
			} else {
				// The left half is bigger.
				if( mean >= 2 ) {
					// Put the left half in stack.
					arrayStack[stackPos] = subArray;
					sizeStack[stackPos] = mean;
					stackPos++;
				}
				if( rightHalfSize >= 2 ) {
					// Rerun the algorithm for the smaller part.
					subArray += mean + 1;
					subArraySize = rightHalfSize;
					continue;
				}
			}
		}
		// We get there if the current sub array consists of a single element or if the sub array is small enough and has been sorted with insertionSort.
		// Time to check the stack for unsorted sub arrays.
		if( stackPos == 0 ) {
			// Done.
			break;
		}
		stackPos--;
		subArray = arrayStack[stackPos];
		subArraySize = sizeStack[stackPos];
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Sort.

}	// namespace Relib.