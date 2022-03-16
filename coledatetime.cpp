#include "coledatetime.h"
#include <sstream>

//using namespace boost::gregorian;

/**
 * COleDateTime
 */



COleDateTime::COleDateTime()
: COleDateTime(1899, 12, 30, 0, 0, 0)
{
}

COleDateTime::COleDateTime(int32_t nYear,
    int32_t nMonth,
    int32_t nDay,
    int32_t nHour,
    int32_t nMin,
    int32_t nSec)
{

    try
    {
        m_status = valid;
        m_Tm.tm_year = nYear - 1900;
        m_Tm.tm_mon = nMonth - 1;
        m_Tm.tm_mday = nDay;
        m_Tm.tm_hour = 0;
        m_Tm.tm_min = 0;
        m_Tm.tm_sec = 0;
        m_status = valid;
    }
    catch (...)
    {
        m_status = error;
    }
}

COleDateTime COleDateTime::GetCurrentTime()
{
    time_t t = time(0);
    struct tm* timeStruct = localtime(&t);
    int32_t year, month, day;
    year = timeStruct->tm_year + 1900;
    month = timeStruct->tm_mon + 1;
    day = timeStruct->tm_mday;

    COleDateTime retDate = COleDateTime(year, month, day,0,0,0);
    return retDate;
}

int32_t COleDateTime::GetYear() const
{
    return m_Tm.tm_year + 1900;
}

int32_t COleDateTime::GetMonth() const
{
    return m_Tm.tm_mon + 1;
}

int32_t COleDateTime::GetDay() const
{
    return m_Tm.tm_mday;
}

int32_t COleDateTime::GetHour() const
{
    
    return 0;
}

int32_t COleDateTime::GetMinute() const
{
    return 0;
}

int32_t COleDateTime::GetDayOfYear() const
{
    int32_t dayofyear;
    struct tm myTM = m_Tm;
    myTM.tm_year += YearShift;  //  Shift year to perform time.h functions
    time_t mytime = mktime(&myTM);
    struct tm* p_myTM = localtime(&mytime);
    myTM = *p_myTM;
    dayofyear = myTM.tm_yday;
    return dayofyear;
}

COleDateTime::DateTimeStatus COleDateTime::GetStatus() const
{
    return m_status;
}

bool COleDateTime::operator < (const COleDateTime& other) const
{
    struct tm thisTime = m_Tm;
    struct tm otherTime = other.m_Tm;
    thisTime.tm_year += YearShift;
    otherTime.tm_year += YearShift;
    return mktime(&thisTime) < mktime(&otherTime);
}

bool COleDateTime::operator > (const COleDateTime& other) const
{
    struct tm thisTime = m_Tm;
    struct tm otherTime = other.m_Tm;
    thisTime.tm_year += YearShift;
    otherTime.tm_year += YearShift;
    return mktime(&thisTime) > mktime(&otherTime);
}

bool COleDateTime::operator >= (const COleDateTime& other) const
{
    struct tm thisTime = m_Tm;
    struct tm otherTime = other.m_Tm;
    thisTime.tm_year += YearShift;
    otherTime.tm_year += YearShift;
    return mktime(&thisTime) >= mktime(&otherTime);
}

bool COleDateTime::operator <= (const COleDateTime& other) const
{
    struct tm thisTime = m_Tm;
    struct tm otherTime = other.m_Tm;
    thisTime.tm_year += YearShift;
    otherTime.tm_year += YearShift;
    return mktime(&thisTime) <= mktime(&otherTime);
}

CString COleDateTime::Format(const char* format) const
{
    //Note - only formats mm/dd/yyyy

    CString datestring;
    std::stringstream ss;
    ss << m_Tm.tm_mon + 1 << "/" << m_Tm.tm_mday << "/" << m_Tm.tm_year + 1900;
    return datestring = ss.str();
}

bool COleDateTime::ParseDateTime(const CString& dateTimeStr, DWORD dwFlags)
{
   //Note:  Only parses mm/dd/yyyy or m/d/yyyy
    std::string month_stg, day_stg, year_stg;
    int pos = 0;
    month_stg = dateTimeStr.Tokenize("/", pos);
    day_stg = dateTimeStr.Tokenize("/", pos);
    year_stg = dateTimeStr.Tokenize("/", pos);
    m_Tm.tm_mon = strtol(month_stg.c_str(), NULL, 10) - 1;
    m_Tm.tm_mday = strtol(day_stg.c_str(), NULL, 10);
    m_Tm.tm_year = strtol(year_stg.c_str(), NULL, 10) - 1900;
    if (IsValidDate()) m_status = valid;
    else m_status = error;
    return (m_status == valid);
}

int COleDateTime::SetDate(int32_t year, int32_t month, int32_t day)
{
    if ((year > 1570) && ((month >0)&&(month<=12)) && ((day >0) && (day <32)))
    {
        m_Tm.tm_year = year - 1900;
        m_Tm.tm_mon = month - 1;
        m_Tm.tm_mday = day;
        m_Tm.tm_hour = 0;
        m_Tm.tm_min = 0;
        m_Tm.tm_sec = 0;
        m_status = valid;
    }
    else
    {
        m_status = error;
    }
    return (m_status == valid);
}

bool COleDateTime::IsLeapYear() const
{
    bool leapyear = false;
    if ((m_Tm.tm_year + 1900) % 4 == 0)
    {
        leapyear = true;
        if (((m_Tm.tm_year + 1900) % 100 == 0) && ((m_Tm.tm_year + 1900) % 400 != 0)) leapyear = false;
    }
    return leapyear;
}

bool COleDateTime::IsYearLeapYear(int32_t year)
{
    bool leapyear = false;
    if (year % 4 == 0)
    {
        leapyear = true; 
        if ((year % 100 == 0) && (year % 400 != 0)) leapyear = false;
    }
    return leapyear;
}


bool COleDateTime::IsValidDate() const
{
    bool status = false;
    // Check to see if date is a valid one
    if ((m_Tm.tm_year + 1900 > 1570))
    {
        if ((m_Tm.tm_mon + 1 == 1) || (m_Tm.tm_mon + 1 == 3)  || (m_Tm.tm_mon + 1 == 5) || (m_Tm.tm_mon + 1 == 7) || (m_Tm.tm_mon + 1 == 8) || (m_Tm.tm_mon + 1 == 10) || (m_Tm.tm_mon + 1 == 12)) // 31 day month
        {
            status = ((m_Tm.tm_mday > 0) && (m_Tm.tm_mday < 32));
        }
        else if (m_Tm.tm_mon + 1 == 2) //February
        {
            if (IsLeapYear()) status = (m_Tm.tm_mday > 0) && (m_Tm.tm_mday < 30);
            else status = (m_Tm.tm_mday > 0) && (m_Tm.tm_mday < 29);
        }
        else status = (m_Tm.tm_mday > 0) && (m_Tm.tm_mday < 31);  // Thirty day month
    }
    return status;
}


COleDateTime COleDateTime::operator+(const COleDateTimeSpan& span) const
{
    COleDateTime dt = *this;

    dt.m_Tm.tm_mday += span.m_day_span;
    dt.m_Tm.tm_year += YearShift;
    time_t temptime = mktime(&dt.m_Tm);
    struct tm* outtm = localtime(&temptime);
    dt.m_Tm = *outtm;
    dt.m_Tm.tm_year -= YearShift;
    return dt;
}

COleDateTime COleDateTime::operator-(const COleDateTimeSpan& span) const
{
    COleDateTime dt = *this;
    dt.m_Tm.tm_mday -= span.m_day_span;
    dt.m_Tm.tm_year += YearShift;
    time_t temptime = mktime(&dt.m_Tm);
    struct tm* outtm = localtime(&temptime);
    dt.m_Tm = *outtm;
    dt.m_Tm.tm_year -= YearShift;
    return dt;
}

COleDateTime& COleDateTime::operator+=(const COleDateTimeSpan& span)
{
    struct tm thisTM = m_Tm;
    thisTM.tm_year += YearShift;
    thisTM.tm_mday += span.m_day_span;
    time_t temptime = mktime(&thisTM);
    struct tm* outtm = localtime(&temptime);
    m_Tm = *outtm;
    m_Tm.tm_year -= YearShift;
    return *this;
}

COleDateTime& COleDateTime::operator-=(const COleDateTimeSpan& span)
{
    struct tm thisTM = m_Tm;
    thisTM.tm_year += YearShift;
    thisTM.tm_mday -= span.m_day_span;
    time_t temptime = mktime(&thisTM);
    struct tm* outtm = localtime(&temptime);
    m_Tm = *outtm;
    m_Tm.tm_year -= YearShift;
    return *this;
}

COleDateTimeSpan COleDateTime::operator-(const COleDateTime& date) const
{
    COleDateTimeSpan dts;
    time_t timeA, timeB;
    double difference;
    struct tm TMA, TMB;
    TMA = m_Tm;
    TMB = date.m_Tm;
    TMA.tm_year += YearShift;
    TMB.tm_year += YearShift;
    timeA = mktime(&TMA);
    timeB = mktime(&TMB);
    difference = difftime(timeA, timeB);
    dts.m_day_span = static_cast<int>(difference / 86400);

    return dts;
}

/**
 * COleDateTimeSpan
 */

COleDateTimeSpan::COleDateTimeSpan()
: m_day_span(0)
{
}

COleDateTimeSpan::COleDateTimeSpan(size_t lDays,
    int32_t nHours,
    int32_t nMins,
    int32_t nSecs)
{
    m_day_span = lDays;
}


int32_t COleDateTimeSpan::GetDays()
{
    return m_day_span;
}

bool COleDateTimeSpan::operator!=(const COleDateTimeSpan& other) const
{
    return m_day_span != other.m_day_span;
}
