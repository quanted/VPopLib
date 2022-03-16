#pragma once
#ifndef COLEDATETIME_CUSTOM_H
#define COLEDATETIME_CUSTOM_H
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "cstring.h"
//#include <boost/date_time/gregorian/gregorian.hpp>

#define GetCurrentTime() GetTickCount()
#define VAR_DATEVALUEONLY ((DWORD)0x00000002) /* return date value */

typedef unsigned long DWORD;

class COleDateTimeSpan;

/**
 * Only supports the necessary interface for the good behavior of VarroaPop
 *   
 * Expects date text to be mm/dd/yyyy 
 * 
 * Only deals with dates, no times.
 * 
 */
class COleDateTime
{
public:
	enum DateTimeStatus
	{
		error = -1,
		valid = 0,
		invalid = 1,    // Invalid date (out of range, etc.)
		null = 2,       // Literally has no value
	};


	COleDateTime();

	COleDateTime(int32_t nYear,
		int32_t nMonth,
		int32_t nDay,
		int32_t nHour,
		int32_t nMin,
		int32_t nSec);

	int32_t GetYear() const;
	int32_t GetMonth() const;
	int32_t GetDay() const;
	int32_t GetHour() const;
	int32_t GetMinute() const;

	int32_t GetDayOfYear() const;

	DateTimeStatus GetStatus() const;

	bool operator < (const COleDateTime& other) const;
	bool operator > (const COleDateTime& other) const;
	bool operator >= (const COleDateTime& other) const;
	bool operator <= (const COleDateTime& other) const;

	CString Format(const char* format) const;
	bool ParseDateTime(const CString& dateTimeStr, DWORD dwFlags = VAR_DATEVALUEONLY);

	int SetDate(int32_t year, int32_t month, int32_t day);


	COleDateTime operator+(const COleDateTimeSpan& span) const;
	COleDateTime operator-(const COleDateTimeSpan& span) const;

	COleDateTime& operator+=(const COleDateTimeSpan& span);
	COleDateTime& operator-=(const COleDateTimeSpan& span);

	COleDateTimeSpan operator-(const COleDateTime& date) const;

	static COleDateTime GetCurrentTime();

private:
//public:
	int YearShift = 400;  // This is used to shift the m_Tm year back and forth so it can represent dates between 1570 and 2600.
	bool IsLeapYear() const;
	static bool IsYearLeapYear(int32_t year);
	bool IsValidDate() const;
	DateTimeStatus m_status;
	tm m_Tm;

};

/**
 * Only supports the necessary interface for the good behavior of VarroaPop
 */
class COleDateTimeSpan
{
public:
	friend class COleDateTime;

	COleDateTimeSpan();

	COleDateTimeSpan(size_t lDays,
		int32_t nHours = 0,
		int32_t nMins = 0,
		int32_t nSecs  = 0);

	int32_t GetDays();

	bool operator!=(const COleDateTimeSpan& other) const;

protected:

	int m_day_span;  // number of days between dates.
};

