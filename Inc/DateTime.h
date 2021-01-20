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

	int GetYear() const
		{ return year; }
	int GetMonth() const
		{ return month; }
	int GetDay() const
		{ return day; }

	int GetHour() const
		{ return hour; }
	int GetMinute() const
		{ return minute; }
	int GetSecond() const
		{ return second; }

	// Current local time.
	static CDateTime Now();
	// Current system time.
	static CDateTime NowSystem();
	// Time difference in seconds.
	__int64 operator-( CDateTime other ) const;

private:
	unsigned short year = 0;
	char month = 0;
	char day = 0;
	char hour = 0;
	char minute = 0;
	char second = 0;

	__int64 getSecondsPassed() const;
	int getDaysPassed( int year, int month, int day ) const;
	bool isLeapYear( int year ) const;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


