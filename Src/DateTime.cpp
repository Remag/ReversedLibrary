#include <DateTime.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CDateTime::CDateTime( int year, int month, int day, int hours, int minutes, int seconds )
{
	const auto monthDaysPassed = getDaysPassed( year, month, day );
	const auto yearDelta = year - 1600;
	const auto fullYearDelta = yearDelta - 1;
	const auto leapYearsPassed = fullYearDelta / 4 - fullYearDelta / 100 + fullYearDelta / 400;
	const auto yearDaysPassed = yearDelta * 365 + leapYearsPassed;
	const auto yearSecondsPassed = static_cast<__int64>( monthDaysPassed + yearDaysPassed ) * 86400;
	const auto daySecondsPassed = seconds + 60 * minutes + 3600 * hours + 86400;
	secondCount = yearSecondsPassed + daySecondsPassed;
}

int CDateTime::getDaysPassed( int year, int month, int day ) const
{
	const int monthDaysPassed[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	return ( month > 3 && isLeapYear( year ) )
		? monthDaysPassed[month] + day
		: monthDaysPassed[month] + day - 1;
}

bool CDateTime::isLeapYear( int year ) const
{
	return ( ( year & 3 ) == 0 ) && ( year % 100 != 0 || year % 400 == 0 );
}

CDateTime CDateTime::Now()
{
	SYSTEMTIME systemTime;
	::GetSystemTime( &systemTime );
	return CDateTime( systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond );
}

__int64 CDateTime::operator-( CDateTime other ) const
{
	return secondCount - other.secondCount;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
