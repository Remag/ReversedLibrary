#include <DateTime.h>
#include <Reassert.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CDateTime::CDateTime( int _year, int _month, int _day, int _hour, int _minute, int _second ) :
	year( static_cast<unsigned short>( _year ) ),
	month( static_cast<char>( _month ) ),
	day( static_cast<char>( _day ) ),
	hour( static_cast<char>( _hour ) ),
	minute( static_cast<char>( _minute ) ),
	second( static_cast<char>( _second ) )
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

int CDateTime::getDaysPassed( int targetYear, int targetMonth, int targetDay ) const
{
	const int monthDaysPassed[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	return ( targetMonth > 3 && isLeapYear( targetYear ) )
		? monthDaysPassed[targetMonth] + targetDay
		: monthDaysPassed[targetMonth] + targetDay - 1;
}

bool CDateTime::isLeapYear( int targetYear ) const
{
	return ( ( targetYear & 3 ) == 0 ) && ( targetYear % 100 != 0 || targetYear % 400 == 0 );
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
