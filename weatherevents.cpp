// WeatherEvents.cpp: implementation of the CWeatherEvents class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "weatherevents.h"
#include "weathergriddata.h"
#include "coldstoragesimulator.h"
#include "globaloptions.h"
#include "math.h"

#include <algorithm>
#include <functional>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct call_if_not_calling
{
	bool calling = false;
	void call(std::function<void()> function)
	{
		if (!calling)
		{
			calling = true;
			function();	
			calling = false;
		}
	}
};

//  Free Functions
int CountChars(CString instg, TCHAR testchar)
{
	//  Returns the number of occurences of testchar in instg

	int count = 0;
	int len = instg.GetLength();

	for (int index=0; index < len; index++) if (instg[index]==testchar) count++;
	return count;
}

// Compute the Forage and ForageInc attributes of a newly created event
void CEvent::UpdateForageAttributeForEvent(double Latitude, double windSpeed)
{
	if (windSpeed == -1) windSpeed = GetWindspeed();  // For default, use the current windspeed
	const bool forageDayBasedOnTemperatures = GlobalOptions::Get().ShouldForageDayElectionBasedOnTemperatures();
	if (forageDayBasedOnTemperatures)
	{
		//
		// Decide if this is a foraging day.  This requires:
		//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
		//    Windspeed <= 8.94 m/s                 AND
		//    Rainfall <= .197 inches or 5 mm
		//
		// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
		SetForage((GetMaxTemp() > 12.0) && (windSpeed <= GlobalOptions::Get().WindspeedThreshold()) &&
			(GetMaxTemp() <= 43.33) && (GetRainfall() <= GlobalOptions::Get().RainfallThreshold()));
	}
	else
	{
		//
		// Decide if this is a foraging day.  This requires:
		//    Windspeed <= 8.94 m/s                AND
		//    Rainfall <= .197 inches or 5 mm
		//
		// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
		SetForage((windSpeed <= GlobalOptions::Get().WindspeedThreshold()) && (GetRainfall() <= GlobalOptions::Get().RainfallThreshold()));
	}
	// Here we set the Forage Increment using the default method, may be change later depending on execution options
	SetHourlyForageInc(Latitude);
}


//////////////////////////////////////////////////////////////////////
// CEvents Member Functions
//////////////////////////////////////////////////////////////////////
CEvent::CEvent()
{
	m_ForageDay = true;
	m_ForageInc = 0.0;
	m_Temp = 0;
	m_MaxTemp = 0;
	m_MinTemp = 0;
	m_Rainfall = 0;
	m_DaylightHours = 0;
}

CEvent::~CEvent()
{
}


CString CEvent::ToString()
{
	CString EventText;
	CString theDate = m_Time.Format("%m/%d/%Y");
	EventText.Format("Date: %s\n Temp: %4.2f\n MaxTemp: %4.2f\n MinTemp: %4.2f\n Rainfall: %4.2f\n DaylightHours: %4.2f\n", 
		theDate, m_Temp, m_MaxTemp, m_MinTemp, m_Rainfall, m_DaylightHours);
	return EventText;
}



CEvent::CEvent(CEvent& event)  // Copy Constructor
{
	m_Time = event.m_Time;
	m_MaxTemp = event.m_MaxTemp;
	m_MinTemp = event.m_MinTemp;
	m_Rainfall = event.m_Rainfall;
	m_ForageDay = event.m_ForageDay;
	m_ForageInc = event.m_ForageInc;
	m_DaylightHours = event.m_DaylightHours;
}

double CEvent::GetTemp()
{
	double pEventTemp = m_Temp;
	static call_if_not_calling caller; 
	caller.call([this, &pEventTemp] {
		const CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
		if (coldStorage.IsEnabled())
			pEventTemp = coldStorage.GetTemp(*this);
	});
	return pEventTemp;
}

double CEvent::GetMaxTemp()
{
	double pEventTemp = m_MaxTemp;
	static call_if_not_calling caller;
	caller.call([this, &pEventTemp] {
		const CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
		if (coldStorage.IsEnabled())
			pEventTemp = coldStorage.GetMaxTemp(*this);
	});
	return pEventTemp;
}

double CEvent::GetMinTemp()
{
	double pEventTemp = m_MinTemp;
	static call_if_not_calling caller;
	caller.call([this, &pEventTemp] {
		const CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
		if (coldStorage.IsEnabled())
			pEventTemp = coldStorage.GetMinTemp(*this);
		});
	return pEventTemp;
}

void CEvent::SetForage(bool Forage)
{
	m_ForageDay = Forage;
}

bool CEvent::IsForageDay()
{
	
	bool forageDay = m_ForageDay;
	return forageDay;
}

double CEvent::GetDaylightHours()
{
	return m_DaylightHours;
}

double CEvent::GetForageInc()
{
	double forageInc = m_ForageInc;
	static call_if_not_calling caller;
	caller.call([this, &forageInc] {
		const CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
		if (coldStorage.IsEnabled())
			forageInc = coldStorage.GetForageInc(*this);
		});
	return forageInc;
}

double CEvent::CalcTodayDaylightFromLatitude(double Lat)
{

	int DayNum = this->GetTime().GetDayOfYear();

	return CalcDaylightFromLatitudeDOY(Lat, DayNum);


}

double CEvent::CalcDaylightFromLatitudeDOY(double Lat, int DayOfYear)
{
	// Reference:  Ecological Modeling, volume 80 (1995) pp. 87-95, called "A Model 
	// Comparison for Daylength as a Function of Latitude and Day of the Year."
	// Lat is in degrees - limited to be between +65 and -65 degrees.  Beyond that, Day is either 24 or 0 hours.
	// Probably not a lot of honeybee colonies inside the arctic and antarctic circles anyway.

	if ((DayOfYear > 366) || (DayOfYear < 1)) return 0.0;

	if (Lat < 0.0)
	{
		Lat = -Lat;
		DayOfYear = (DayOfYear + 182) % 365;
	}
	if (Lat > 65.0) Lat = 65.0;

	double PI = 3.14159265358979;
	double DaylightHours = 0;

	double P = asin(0.39795 * cos(0.2163108 + 2 * atan(0.9671396 * tan(0.00860 * (DayOfYear - 186)))));
	DaylightHours =
		24 - (24 / PI) * acos((sin(0.833 * PI / 180) + sin(Lat * PI / 180) * sin(P)) / cos(Lat * PI / 180) * cos(P));
	return DaylightHours;
}


CEvent CEvent:: operator = (CEvent& event)
{
	CEvent temp;
	m_Time = event.m_Time;
	m_MaxTemp = event.m_MaxTemp;
	m_MinTemp = event.m_MinTemp;
	m_Rainfall = event.m_Rainfall;
	m_ForageDay = event.m_ForageDay;
	m_ForageInc = event.m_ForageInc;
	m_DaylightHours = event.m_DaylightHours;
	return temp;
}


CEvent CEvent::operator + (CEvent event)
{
	/*  
	When adding events, increment rainfall, daylight hours; reset min and 
	max temp if necessary;  If the either event is not a forage day, the sum is not a 
	forage day.  Otherwise, if for either event daylight and wind>25 or temp<50 or temp > 110 or rain, the
	sum is not a foraging day.  Keep time as it is since this corresponds the the beginning of the
	event.
	*/

	CEvent temp;
	temp.m_Time = m_Time;
	temp.m_Rainfall = m_Rainfall + event.m_Rainfall;
	temp.m_DaylightHours = m_DaylightHours + event.m_DaylightHours;
	temp.m_MaxTemp = (m_MaxTemp > event.m_MaxTemp)?m_MaxTemp:event.m_MaxTemp;
	temp.m_MinTemp = (m_MinTemp < event.m_MinTemp)?m_MinTemp:event.m_MinTemp;
	temp.m_ForageDay = (m_ForageDay && event.m_ForageDay);
	temp.m_ForageInc = m_ForageInc + event.m_ForageInc;
	return temp;
}
	

void CEvent::operator += (CEvent event)
{
	m_Rainfall += event.m_Rainfall;
	m_DaylightHours += event.m_DaylightHours;
	m_MaxTemp = (m_MaxTemp > event.m_MaxTemp)?m_MaxTemp:event.m_MaxTemp;
	m_MinTemp = (m_MinTemp < event.m_MinTemp)?m_MinTemp:event.m_MinTemp;
	m_ForageDay = (m_ForageDay && event.m_ForageDay);
	m_ForageInc += event.m_ForageInc;
}


CString CEvent::GetDateStg(CString formatstg)
{
	CString DateStg;
	DateStg = m_Time.Format(formatstg);
	return DateStg;
}

bool CEvent::IsWinterDay()
{
	bool WD;
	WD = m_Temp < 18.0;  //Try this out for now
	return WD;
}


void CEvent::SetHourlyForageInc(double Latitude)
{
		// Calculate the daylight hours for the longest day of the year at this latitude - this is the solstice

		double solstice = CalcDaylightFromLatitudeDOY(Latitude, 182) + 1;  // Length of longest day plus one hour of flying time before sunrise
		if (solstice <= 0)
		{
			SetForageInc(0);
			return;
		}

		double todaylength = CalcDaylightFromLatitudeDOY(Latitude, GetTime().GetDayOfYear());

		//Now estimate the number of hours of daylight today within the temperature thresholds
		double flightdaylength = CalcFlightDaylight(todaylength);

		//Now set ForageInc as a proportion of flying daylight hours to solstice daylight hours
		const double FInc = (flightdaylength / solstice) > 1.0 ? 1.0 : (flightdaylength / solstice);
		SetForageInc(FInc);

}

double CEvent::CalcFlightDaylight(double daylength, double MinTempThreshold, double MaxTempThreshold)
{
	// This returns the number of hours this day that are in daylight and within the temperature thresholds
	/*
	From:  New algorithm for generating hourly temperaturevalues using daily maximum, minimum and average values 
	rom climate modelsDHC Chow BSc BEng PhD MCIBSE MASHRAEand Geoff J Levermore BSc ARCS PhD DIC CEng DMSFCIBSE MASHRAE
	*/
	int Sunrise = 12 - (daylength / 2);  //  Sunrise is half the daylight period before noon
	int Sunset = Sunrise + daylength;
	int TimeTMin = Sunrise - 1;  // Typically
	int TimeTMax = 14;  // Two hours after noon
	double TMax = m_MaxTemp;
	double TMin = m_MinTemp;

	// Now calculate the number of hours of daylength that are within the temperature thresholds.
	int flighthours = 0;
	double HrTemp;
	for (int i = TimeTMin; i <= Sunset; i++)
	{
		// Estimate temperature at this time
		HrTemp = (TMax + TMin) / 2 - ((TMax - TMin) / 2 * cos(3.1416 * (i - TimeTMin) / (TimeTMax - TimeTMin)));
		if ((HrTemp > MinTempThreshold) && (HrTemp < MaxTempThreshold))
		{
			flighthours++;
		}
	}
	return double(flighthours);
}



void CEvent::SetForageInc(double TThresh, double TMax, double TAve)
{

	/*  Calculates a measure of the amount of foraging for days having 
		maximum temperatures close to the foraging threshold.
	*/

	double DaytimeRange = TMax - TAve;
	double PropThreshold = (TThresh - TAve)/DaytimeRange;

	if (TMax < TThresh) m_ForageInc = 0.0;
	else if ((0.968<=PropThreshold)&&(PropThreshold<1)) m_ForageInc = 0.25;
	else if ((0.866<=PropThreshold)&&(PropThreshold<.968)) m_ForageInc = 0.5;
	else if ((0.660<=PropThreshold)&&(PropThreshold<.866)) m_ForageInc = 0.75;
	else m_ForageInc = 1.0;
}
//

//////////////////////////////////////////////////////////////////////
// CWeatherEvents Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWeatherEvents::CWeatherEvents()
{
	m_Filename = "";
	m_HasBeenInitialized = false;
	m_Latitude = 30.0;  //Default value
}

CWeatherEvents::~CWeatherEvents()
{
	ClearAllEvents();
}


////////////////////////////////////////////////////////////////////
// CWeatherEvents Member Functions
////////////////////////////////////////////////////////////////////


void CWeatherEvents::ClearAllEvents()
{
	while (!m_EventList.IsEmpty())
	{
		CEvent* temp = m_EventList.RemoveHead();
		ASSERT(temp);
		delete temp;
	}
	if (!m_EventList.IsEmpty()) m_EventList.RemoveAll();
	m_HasBeenInitialized = false;
}

bool CWeatherEvents::AddEvent(CEvent* theEvent)
{
	//  Assumption is that weather events are added sequentially from
	//  the weather file in ascending order of time. Therefore, each event is added
	//  to the tail of the m_EventList.  The validity of this assumption is 
	//  determined later by the CheckSanity() function.

	pos = m_EventList.AddTail(theEvent);
	return true;
}

bool CWeatherEvents::RemoveCurrentEvent()
{
	ASSERT(!m_EventList.IsEmpty());
	CEvent* temp = m_EventList.GetAt(pos);
	ASSERT(temp);
	delete temp;
	m_EventList.RemoveAt(pos);
	return true;
}


int CWeatherEvents::CheckInterval()
{
	//  Verifies that the time interval between each event in m_EventList is equal
	//  to the time interval between the first two events in m_EventList.  If an error
	//  occurs the line number is returned.  If no error, 0 is returned

	ASSERT(!m_EventList.IsEmpty());
	pos = m_EventList.GetHeadPosition();
	CEvent* start = m_EventList.GetHead();
	CEvent* temp = m_EventList.GetNext(pos);
	COleDateTimeSpan diff = temp->m_Time - start->m_Time;
	CEvent* lastEvent = temp;
	while (pos!=NULL)
	{
		temp = m_EventList.GetNext(pos);
		if (diff != (temp->m_Time - lastEvent->m_Time)) return temp->GetLineNum(); 
		else lastEvent = temp;
	}
	return 0;
}

COleDateTime CWeatherEvents::GetBeginningTime()
{
	ASSERT(!m_EventList.IsEmpty());
	CEvent* event = m_EventList.GetHead();
	COleDateTime theTime = event->GetTime();
	TRACE("Read Begining Time from head of eventList = %s\n", theTime.Format("%m/%d/%Y"));
	return event->GetTime();
}



COleDateTime CWeatherEvents::GetEndingTime()
{
	ASSERT(!m_EventList.IsEmpty());
	CEvent* event = m_EventList.GetTail();
	COleDateTime theTime = event->GetTime();
	TRACE("Read ENDING Time from head of eventList = %s\n", theTime.Format("%m/%d/%Y"));
	return event->GetTime();
}


COleDateTime CWeatherEvents::GetCurrentTime()
{
	ASSERT(!m_EventList.IsEmpty());
	CEvent* event = m_EventList.GetAt(pos);
	return event->GetTime();
}


int CWeatherEvents::GetCurrentLineNumber()
{
	ASSERT(!m_EventList.IsEmpty());
	CEvent* event = m_EventList.GetAt(pos);
	return event->GetLineNum();
}


int CWeatherEvents::GetTotalEvents()
{
	return m_EventList.GetCount();
}


int CWeatherEvents::CheckSanity()
{
	return 0;
}


void CWeatherEvents::GoToFirstEvent()
{
	pos = m_EventList.GetHeadPosition();
}

CEvent* CWeatherEvents::GetFirstEvent()
{
	GoToFirstEvent();
	CEvent* pEvent = m_EventList.GetNext(pos);  //Fixed this - was GetAt which didn't iterate properly and caused duplicate first items
	return pEvent;
}

CEvent* CWeatherEvents::GetNextEvent()
{
	if (pos == NULL) return NULL;
	else return(m_EventList.GetNext(pos));

}


CEvent* CWeatherEvents::GetDayEvent(COleDateTime theTime)
{
	POSITION oldposition = pos;
	CEvent* tempEvent;
	GoToFirstEvent();
	while ((tempEvent = GetNextEvent())!=NULL)
	{
		if ((tempEvent->GetTime().GetYear() == theTime.GetYear())&&
			(tempEvent->GetTime().GetMonth() == theTime.GetMonth())&&
			(tempEvent->GetTime().GetDay() == theTime.GetDay()))
			return tempEvent;
	}
	pos = oldposition;
	return NULL;
}

struct GridDataTypeId
{
	static const char Observed[];
	static const char Modeled[];
	static const char Rcp85[];
	static const char Rcp45[];
};

const char GridDataTypeId::Observed[] = "Observed";
const char GridDataTypeId::Modeled[] = "Modeled";
const char GridDataTypeId::Rcp85[] = "Rcp85";
const char GridDataTypeId::Rcp45[] = "Rcp45";
