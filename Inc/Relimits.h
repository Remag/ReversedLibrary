#pragma once
#include <Redefs.h>
#include <limits.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Numeric limits information.
template <class T>
struct CLimits {};

template<>
struct CLimits<char> {
	static const char Max = CHAR_MAX;
	static const char Min = CHAR_MIN;
	static const int MaxDigitsBase10 = 3;
};

template<>
struct CLimits<short> {
	static const short Max = SHRT_MAX;
	static const short Min = SHRT_MAX;
	static const int MaxDigitsBase10 = 5;
};

template<>
struct CLimits<int> {
	static const int Max = INT_MAX;
	static const int Min = INT_MIN;
	static const int MaxDigitsBase10 = 10;
};

template<>
struct CLimits<unsigned> {
	static const unsigned Max = UINT_MAX;
	static const int Min = 0;
	static const int MaxDigitsBase10 = 10;
	static const int MaxDigitsBase16 = 8;
};

template<>
struct CLimits<__int64> {
	static const __int64 Max = INT64_MAX;
	static const __int64 Min = INT64_MIN;
	static const int MaxDigitsBase10 = 19;
};

template<>
struct CLimits<float> {
	static const REAPI float Max;
	static const REAPI float Min;

	typedef int SameSizedIntType;
};

template<>
struct CLimits<double> {
	static const REAPI double Max;
	static const REAPI double Min;

	typedef __int64 SameSizedIntType;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

