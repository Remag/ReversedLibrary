#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Date representation. Time is stored with a second precision.
// Only dates past year 1600 are guaranteed to be properly represented.
// Leap seconds are not accounted for.
class REAPI CDateTime {
public:
	CDateTime() = default;
	CDateTime( int year, int month, int day, int hours, int minutes, int seconds );

	// Current system time.
	static CDateTime Now();
	// Time difference in seconds.
	__int64 operator-( CDateTime other ) const;

private:
	// Time is internally stored as the number of seconds since 1600-1-1T0:00Z ( leap seconds are ignored ).
	__int64 secondCount = 0;

	int getDaysPassed( int year, int month, int day ) const;
	bool isLeapYear( int year ) const;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


