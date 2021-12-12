#include "coledatetime.h"
#include <sstream>

using namespace boost::gregorian;

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
        m_date = boost::gregorian::date(nYear, nMonth, nDay);
        m_status = valid;
    }
    catch (...)
    {
        m_status = error;
    }
}

COleDateTime COleDateTime::GetCurrentTime()
{
	//return date.current_date(boost::gregorian::day_clock::local_day());
    boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
    int32_t year, month, day;
    year = current_date.year();
    month = current_date.month();
    day = current_date.day();

    COleDateTime retDate = COleDateTime(year, month, day,0,0,0);
    return retDate;
}

int32_t COleDateTime::GetYear() const
{
    return m_date.year();
}

int32_t COleDateTime::GetMonth() const
{
    return m_date.month();
}

int32_t COleDateTime::GetDay() const
{
    return m_date.day();
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
    return m_date.day_of_year();
}

COleDateTime::DateTimeStatus COleDateTime::GetStatus() const
{
    return m_status;
}

bool COleDateTime::operator < (const COleDateTime& other) const
{
    return m_date < other.m_date;
}

bool COleDateTime::operator > (const COleDateTime& other) const
{
    return m_date > other.m_date;
}

bool COleDateTime::operator >= (const COleDateTime& other) const
{
    return m_date >= other.m_date;
}

bool COleDateTime::operator <= (const COleDateTime& other) const
{
    return m_date <= other.m_date;
}

CString COleDateTime::Format(const char* format) const
{
    CString string;
    date_facet* df = new  date_facet{ format };
    std::stringstream ss;
    ss.imbue(std::locale{ std::cout.getloc(), df });
    ss << m_date;
    string = ss.str();
    return string;
}

bool COleDateTime::ParseDateTime(const CString& dateTimeStr, DWORD dwFlags)
{
    try
    {
        m_date = from_us_string(dateTimeStr.ToString());
        return m_status == valid;
    }
    catch (...)
    {
        return m_status == error;
    }
}

int COleDateTime::SetDate(int32_t year, int32_t month, int32_t day)
{
    try
    {
        m_date = date(year, month, day);
        m_status = valid;
    }
    catch (...)
    {
        m_status = error;
    }

    return (m_status == valid);
}



COleDateTime COleDateTime::operator+(const COleDateTimeSpan& span) const
{
    COleDateTime dt;
    dt.m_date = m_date + span.m_day_span;
    return COleDateTime(dt);
}

COleDateTime COleDateTime::operator-(const COleDateTimeSpan& span) const
{
    COleDateTime dt;
    dt.m_date = m_date - span.m_day_span;
    return COleDateTime(dt);
}

COleDateTime& COleDateTime::operator+=(const COleDateTimeSpan& span)
{
    m_date += span.m_day_span;
    return *this;
}

COleDateTime& COleDateTime::operator-=(const COleDateTimeSpan& span)
{
    m_date -= span.m_day_span;;
    return *this;
}

COleDateTimeSpan COleDateTime::operator-(const COleDateTime& date) const
{
    COleDateTimeSpan dts;
    dts.m_day_span = m_date - date.m_date;   
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
    int32_t nHours = 0,
    int32_t nMins = 0,
    int32_t nSecs = 0)
{
    m_day_span = date_duration(lDays);
}


int32_t COleDateTimeSpan::GetDays()
{
    return m_day_span.days();
}

bool COleDateTimeSpan::operator!=(const COleDateTimeSpan& other) const
{
    return m_day_span != other.m_day_span;
}
