#include <DateTime.h>
#include <Reassert.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CDateTime::CDateTime( int _year, int _month, int _day, int _hour, int _minute, int _second ) :
	year( _year ),
	month( _month ),
	day( _day ),
	hour( _hour ),
	minute( _minute ),
	second( _second )
{
	// Win API imposed year restrictions.
	assert( _year >= 1601 && _year < 30827 );
	assert( _month >= 1 && _month <= 12 );
	assert( _day >= 1 && _day <= 31 );
	assert( _hour >= 0 && _hour <= 23 );
	assert( _minute >= 0 && _minute <= 59 );
	assert( _second >= 0 && _second <= 59 );
}

__int64 CDateTime::getSecondsPassed() const
{
	const auto monthDaysPassed = getDaysPassed( year, month, day );
	const auto yearDelta = year - 1600;
	const auto fullYearDelta = yearDelta - 1;
	const auto leapYearsPassed = fullYearDelta / 4 - fullYearDelta / 100 + fullYearDelta / 400;
	const auto yearDaysPassed = yearDelta * 365 + leapYearsPassed;
	const auto yearSecondsPassed = static_cast<__int64>( monthDaysPassed + yearDaysPassed ) * 86400;
	const auto daySecondsPassed = second + 60 * minute + 3600 * hour + 86400;
	return yearSecondsPassed + daySecondsPassed;
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
	SYSTEMTIME localTime;
	::GetLocalTime( &localTime );
	return CDateTime( localTime.wYear, localTime.wMonth, localTime.wDay, 
		localTime.wHour, localTime.wMinute, localTime.wSecond );
}

CDateTime CDateTime::NowSystem()
{
	SYSTEMTIME systemTime;
	::GetSystemTime( &systemTime );
	return CDateTime( systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond );
}

__int64 CDateTime::operator-( CDateTime other ) const
{
	return getSecondsPassed() - other.getSecondsPassed();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
