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
	//SetForageInc(12.0, GetMaxTemp(), GetTemp());
	SetHourlyForageInc(Latitude);
	//m_ForageInc = 1.0;  // NOTE:  test only - remove after test
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
	//static call_if_not_calling caller;
	//caller.call([this, &forageDay] {
	//	const CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
	//	if (coldStorage.IsEnabled())
	//		forageDay = coldStorage.IsForageDay(*this);
	//	});
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

		//const double todaylength = CalcTodayDaylightFromLatitude(Latitude);
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
//CString CWeatherEvents::GetFileName()
//{
//	return m_Filename;
//
//}

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


//*** Legacy File Access methods *** - may be useful at some point for reference
//
//
//
//
//bool CWeatherEvents::LoadWeatherFile(CString FileName)
//{
//  
//	// Open the weather file.  FileName is the fully qualified path for the weather file
//	
//	CString Extension = SplitPath(FileName,EXT);
//	if (Extension.MakeUpper() == ".DVF") // EPA Weather files have this extension
//	{
//		return(LoadEPAWeatherFileDVF(FileName));
//	}
//	else if (Extension.MakeUpper() == ".WEA")
//	{
//		return(LoadEPAWeatherFileWEA(FileName));
//	}
//	else
//	{
//		// Check if the file is in Binary
//		const std::string binaryIdentifier = GlobalOptions::Get().BinaryWeatherFileFormatIdentifier();
//		if (!binaryIdentifier.empty())
//		{
//			// loading binary weather 
//			if (binaryIdentifier == GridDataTypeId::Observed)
//				LoadWeatherGridDataBinaryFile<ObservedHistoricalItem>(FileName);
//			else if (binaryIdentifier == GridDataTypeId::Modeled)
//				LoadWeatherGridDataBinaryFile<ModeledHistoricalItem>(FileName);
//			else if (binaryIdentifier == GridDataTypeId::Rcp85)
//				LoadWeatherGridDataBinaryFile<Rcp85>(FileName);
//			else if (binaryIdentifier == GridDataTypeId::Rcp45)
//				LoadWeatherGridDataBinaryFile<Rcp45>(FileName);
//			else
//			{
//				CString stg = "Error Binary Weather File Identifier Not Supported: ";
//				MyMessageBox(stg + binaryIdentifier.c_str());
//				return false;  //  Header Loading Failed
//			}
//		}
//		else
//		{
//			// Load legacy VarroaPop weather files .wth
//			CWeatherFile theFile;
//			CFileException ex;
//			if (!theFile.Open(FileName, CFile::modeRead | CFile::shareDenyNone, &ex))
//			{
//				TCHAR   szCause[255];
//				CString strFormatted;
//				ex.GetErrorMessage(szCause, 255);
//				strFormatted = _T("The data file could not be opened because of this error: ");
//				strFormatted += szCause;
//				//MyMessageBox(strFormatted);
//				return false;
//			}
//
//			// Clear the Weather Event List;
//			ClearAllEvents();
//
//			// Does the file have a header?
//			if (!theFile.IsHeaderPresent())
//			{
//				if ((MyMessageBox("This Weather file has no Header \n Would you Like to Create One?",
//					MB_YESNO) == IDNO)) return false;
//				if (!theFile.CreateHeader())
//				{
//					CString msg = "Error reading file header: ";
//					if (theFile.GetError().GetLength() != 0) MyMessageBox(msg + theFile.GetError());
//					return false;  //  Header Construction Failed
//				}
//			}
//
//			// Load the Header info into theFile object
//			if (!theFile.DigestHeader())
//			{
//				CString stg = "Error Reading Weather File Header: ";
//				MyMessageBox(stg + theFile.GetError());
//				return false;  //  Header Loading Failed
//			}
//
//			// Prepare Progress Bar
//			int Step = 0;
//			float val = 0.0;
//			CDialog ProgressDlg;
//			CPoint Offset(10, 10);
//			CProgressCtrl* pProgress;
//			if (gl_RunGUI)
//			{
//				ProgressDlg.Create(IDD_PROGRESS);
//				ProgressDlg.SetWindowText("Loading Weather File " + FileName);
//				pProgress = (CProgressCtrl*)ProgressDlg.GetDlgItem(IDC_PROGRESS);
//				if (theFile.m_Size != 0) val = float(10) / theFile.m_Size;
//				else val = 0;
//			}
//
//			std::vector<CEvent*> events;
//
//			// Now Process the Data
//			CEvent* pEvent = theFile.GetNextEvent();
//			while (pEvent != NULL)
//			{
//				if ((val > float(Step) / theFile.m_BytesRead) && (gl_RunGUI))
//				{
//					pProgress->StepIt();
//					Step++;
//				}
//				//TRACE("Adding a weather event \n");
//				AddEvent(pEvent);
//				events.push_back(pEvent);
//				TRACE("Weather Date: %s\n", pEvent->GetDateStg());
//				pEvent = theFile.GetNextEvent();
//			}
//			if (gl_RunGUI) ProgressDlg.DestroyWindow();
//
//			SetFileName(FileName);
//
//			if (GlobalOptions::Get().ShouldComputeHourlyTemperatureEstimation())
//			{
//				ComputeHourlyTemperatureEstimationAndUpdateForageInc(events);
//			}
//		}
//	}
//
//	HasBeenInitialized = true;
//	return true;
//}
//	
//	
//bool CWeatherEvents::LoadEPAWeatherFileDVF(CString FileName)
///***********************************************************************************
//The EPA Daily Weather files are described as follows
//
//Station WBAN Number: 03103
//Station Name: Flagstaff
//Station Location (State): AZ
//Station Time Zone: +7(T)
//Station Latitude:  N   35 degrees  8 minutes
//Station Longitude: W  111 degrees 40 minutes
//Station elevation above sea level [meters]: 2135
//File generation date: 2002-07-02 16:28:53
//Years present: 1961-1990
//!
//! The meteorological Daily Values File "w03103.dvf" has the following format:
//!
//! Field  Columns   Description            Units                    Type        Format
//! -----  --------  ---------------------- ---------------          ---------   ------
//!   1        1     blank                  N/A                      Character   1x
//!   2     02 - 07  Date                   mmddyy                   Integer     3i2
//!   3     08 - 17  Precipitation          cm/day                   Real        f10.2
//!   4     18 - 27  Pan Evaporation        cm/day                   Real        f10.2
//!   5     28 - 37  Temperature (mean)     degrees Centigrade       Real        f10.1
//!   6     38 - 47  Wind Speed @10 meter   cm/second                Real        f10.1
//!   7     48 - 57  Solar Radiation        Langleys/day             Real        f10.1
//!   8     58 - 63  FAO Short Grass Eto    mm/day                   Real        f6.1
//!   9     64 - 73  Daylight Station       kiloPascal               Real        f10.1
//!                    Pressure
//!  10     74 - 77  Daylight Relative      percent                  Integer     i4
//!                    Humidity
//!  11     78 - 80  Daylight Opaque        tenths of sky covered    Integer     i3
//!                    Sky Cover
//!  12     81 - 90  Daylight Temperature   degrees Centigrade       Real        f10.1
//!  13     91 - 96  Daylight Broadband     optical depth            Real        f6.3
//!                    Aerosol
//!  14     97 -102  Daylight Prevailing    meter/second             Real        f6.1
//!                    Wind Speed @10 meter
//!  15    103 -106  Daylight Prevailing    degrees (N=0, E=90, ...) Integer     i4
//!                    Wind Direction
//!
//! Daily values file format: (1x,3i2, t8,f10.2, t18,f10.2, t28,f10.1, t38,f10.1, &
//!                           t48,f10.1, t58,f6.1, t64,f10.1, t74,i4, t78,i3, &
//!                           t81,f10.1, t91,f6.3, t97,f6.1, t103,i4)
//!
//! Fields 03 - 08 represent daily totals or mean values (Wind Speed and Temperature)
//! Fields 03 - 07 units preserved from earlier PRZM met files for compatibility
//! Fields 09 - 15 represent mean values for daylight hours only.
//*/
//
///*
//!	Note:  For EPA ".dxf" files there are corresponding ".txt" files which contain 
//!	headers like the one in the comment box directly above.  These headers contain
//!	the Weather Station Latitude which is used to calculate daylight hours.  This 
//!	function looks for the corresponding ".txt" file in the same directory as the ".dxf"
//!	file  and if found will extract the latitude info.  If not found, a latitude
//!	of N 30 degrees is assumed.
//*/
//
///*
//	The EPA weather files do not have min and max temperature but they do have 24 hour mean and daylight
//	mean.  We estimate the MinTemp and MaxTemp as follows (using sine wave approximation for daily temperatures):
//	
//	Peak Delta = (Daylight Mean - Daily Mean)/0.637
//	MaxTemp = Daily Mean + Peak Delta
//	MinTemp = Daily Mean - Peak Delta
//*/
//{
//		CWeatherFile theFile;
//		double Latitude;
//		CFileException ex;
//		if (!theFile.Open(FileName,CFile::modeRead|CFile::shareDenyNone, &ex))
//		{
//			TCHAR   szCause[255];
//			CString strFormatted;
//			ex.GetErrorMessage(szCause, 255);
//			strFormatted = _T("The data file could not be opened because of this error: ");
//			strFormatted += szCause;
//			//MyMessageBox(strFormatted);
//			return false;
//		}
//
//		Latitude = GetLatitudeFromFile(FileName);
//			
//		// Clear the Weather Event List;
//		ClearAllEvents();
//
//
//		// Prepare Progress Bar
//		int Step = 0;
//		float val = 0.0;
//		CPoint Offset(10,10);
//		CDialog ProgressDlg;
//		CProgressCtrl* pProgress;
//		if (gl_RunGUI)
//		{
//			ProgressDlg.Create(IDD_PROGRESS);
//			ProgressDlg.SetWindowText("Loading Weather File "+FileName);
//			pProgress = (CProgressCtrl*)ProgressDlg.GetDlgItem(IDC_PROGRESS);
//			if (theFile.m_Size != 0) val = float(10)/theFile.m_Size;
//			else val = 0;
//		}
//
//		 
//		CString InString;  
//		int LineNum = 1;
//		while (theFile.GetLine(InString))  // Read all lines in the weather File
//		{
//			// Update the Progress bar
//			if ((val > float(Step)/theFile.m_BytesRead)&&(gl_RunGUI))
//				{
//					pProgress->StepIt();
//					Step++;
//				}
//
//			CEvent *pEvent = new CEvent();
//			int curPos = 1;
//			CString ResToken;
//			CStringArray TokenArray;
//			ResToken = InString.Tokenize(" ",curPos);
//			while (ResToken != "") // Load all tokens in to a CStringArray for conversions
//			{
//				TokenArray.Add(LPCTSTR(ResToken));
//				ResToken = InString.Tokenize(" ",curPos);
//			}
//			// Now load tbe pEvent
//			// Date
//			int day, month, year;
//			month = atoi(TokenArray[0].Left(2));
//			day = atoi(TokenArray[0].Mid(2,2));
//			year = atoi(TokenArray[0].Right(2));
//			if ((year < 70) && (year > 37)) // This is a filter to skip ranges beyond COleDateTime - need to convert all CTimes to COLEDateTimes
//			{
//				delete pEvent;  // no memory leaks - delete pEvent just created
//			}
//			else
//			{
//				// File data used 2-digit years.  COleDateTime doesn't recognize dates prior to 1970
//				if (year >= 70) year += 1900;
//				else year += 2000;
//				// Validate date or flag error
//				COleDateTime dateCheck;
//				
//				if (dateCheck.SetDate(year,month,day) == 1)  // check valid date
//				{
//					MyMessageBox("Invalid date format in weather file: "+FileName);
//					return false;
//				}
//				
//					//COleDateTime(year, month, day, 0,0,0);
//					pEvent->SetTime(COleDateTime(year, month, day, 0,0,0));
//				//
//				// Temperatures
//				pEvent->SetTemp(atof(TokenArray[3])); // Mean temperature for 24 hour period
//				double DaylightTemp = atof(TokenArray[10]); // Mean daylight temperature 
//				double PeakDelta = (DaylightTemp - pEvent->GetTemp())/0.637; // Convert from daylight mean to peak - Assume sine wave temp approximation
//				pEvent->SetMaxTemp(pEvent->GetTemp() + PeakDelta);
//				pEvent->SetMinTemp(pEvent->GetTemp() - PeakDelta);
//				
//				// Rainfall
//				pEvent->SetRainfall(atof(TokenArray[1]) / 2.54);  // Convert from cm to inches
//				
//				// Decide if this is a foraging day.  This requires:
//				//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
//				//    Windspeed <= 8.94 m/s                AND
//				//    Rainfall <= .197 inches
//				//
//				// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
//				double windspeed = atof(TokenArray[12]);
//				pEvent->SetForage(
//					(pEvent->GetMaxTemp() > 12.0) && 
//					(windspeed <= GlobalOptions::Get().WindspeedThreshold()) &&
//					(pEvent->GetMaxTemp() <= 43.33) && 
//					(pEvent->GetRainfall() < GlobalOptions::Get().RainfallThreshold()));
//					
//				pEvent->SetForageInc(12.0, pEvent->GetMaxTemp(), pEvent->GetTemp());
//
//				// TEMPORARY until we get the transform function from EPA data.  For now, assume 30 degree latitude
//				double DayHours = CalcDaylightFromLatitude(Latitude, pEvent->m_Time.GetDayOfYear());
//				pEvent->SetDaylightHours(DayHours);  
//
//				pEvent->m_LineNum = LineNum++;
//				AddEvent(pEvent); 
//			}
//		}  
//
//	if (gl_RunGUI) ProgressDlg.DestroyWindow();
//	SetFileName(FileName);
//	HasBeenInitialized = true;
//	return true;
//
//}
//
//bool CWeatherEvents::LoadEPAWeatherFileWEA(CString FileName)
///***********************************************************************************
//The EPA .wea Weather files are described as follows
//
//The weather files here were created per methods in Fry et al (2016); (see Fry_etal.pdf).
//Formatting is given in the supporting material (see SupportingMaterial_Fryetal.pdf).
//Briefly, the comma-separated fields are:
//mm, dd, yyyy, precipitation (cm/day), ETo (cm/day), mean daily temperature (C), wind speed (cm/s), solar radiation (La/day)
//
//The .wea filename format is xxxxx_grid_yy.yyy_lat.wea.  The xxxxx string is an identifier which is used to map the weather station to 
//the longitude of the station but we don't need that for this application.  The yy.yyy string is the weather station latitude (assumed to be N) which is
//used to calculate hours of sunlight per day.  If a valid latitude is not found, a latitude of N 30 degrees is assumed.  
//
//Reference
//Fry, M.M., Rothman, G., Young, D.F., and Thurman, N., 2016.  Daily gridded weather for exposure modeling, Environmental Modelling & Software, 82, 167-173, doi.org/10.1016/j.envsoft.2016.04.008
//
//
//
//*/
//{
//		CWeatherFile theFile;
//		double Latitude;
//		CFileException ex;
//		if (!theFile.Open(FileName,CFile::modeRead|CFile::shareDenyNone, &ex))
//		{
//			TCHAR   szCause[255];
//			CString strFormatted;
//			ex.GetErrorMessage(szCause, 255);
//			strFormatted = _T("The data file could not be opened because of this error: ");
//			strFormatted += szCause;
//			//MyMessageBox(strFormatted);
//			return false;
//		}
//
//		Latitude = GetLatitudeFromFileName(FileName);
//			
//		// Clear the Weather Event List;
//		ClearAllEvents();
//
//
//		// Prepare Progress Bar
//		int Step = 0;
//		float val = 0.0;
//		CPoint Offset(10,10);
//		CDialog ProgressDlg;
//		CProgressCtrl* pProgress;
//		if (gl_RunGUI)
//		{
//			ProgressDlg.Create(IDD_PROGRESS);
//			ProgressDlg.SetWindowText("Loading Weather File "+FileName);
//			pProgress = (CProgressCtrl*)ProgressDlg.GetDlgItem(IDC_PROGRESS);
//			if (theFile.m_Size != 0) val = float(10)/theFile.m_Size;
//			else val = 0;
//		}
//
//		 
//		CString InString;  
//		int LineNum = 1;
//		while (theFile.GetLine(InString))  // Read all lines in the weather File
//		{
//			// Update the Progress bar
//			if ((val > float(Step)/theFile.m_BytesRead)&&(gl_RunGUI))
//				{
//					pProgress->StepIt();
//					Step++;
//				}
//
//			CEvent *pEvent = new CEvent();
//			int curPos = 0;
//			CString ResToken;
//			CStringArray TokenArray;
//			ResToken = InString.Tokenize(",",curPos);  // Comma-separated variables
//			while (ResToken != "") // Load all tokens in to a CStringArray for conversions
//			{
//				TokenArray.Add(LPCTSTR(ResToken));
//				ResToken = InString.Tokenize(",",curPos);
//			}
//			// Now load the pEvent
//			// Date
//			int day, month, year;
//			CString smonth = TokenArray[0];
//			CString sday = TokenArray[1];
//			CString syear = TokenArray[2];
//			TRACE("day: %s,  month: %s, year: %s\n",sday, smonth, syear);
//			
//			CString stg = TokenArray[3];
//			stg = TokenArray[4];
//			stg = TokenArray[5];
//			stg = TokenArray[6];
//
//			//month = atoi(TokenArray[0]);
//			//day = atoi(TokenArray[1]);
//			//year = atoi(TokenArray[2]);
//			month = atoi(smonth);
//			day = atoi(sday);
//			year = atoi(syear);
//			//if ((year < 70) && (year > 37)) // This is a filter to skip ranges beyond COleDateTime - need to convert all CTimes to COLEDateTimes
//			//{
//			//	delete pEvent;  // no memory leaks - delete pEvent just created
//			//}
//			//else
//			{
//				// Validate date or flag error
//				COleDateTime dateCheck;
//				
//				if (dateCheck.SetDate(year,month,day) == 1)  // check valid date
//				{
//					MyMessageBox("Invalid date format in weather file: "+FileName);
//					return false;
//				}
//				
//					//COleDateTime(year, month, day, 0,0,0);
//					pEvent->SetTime(COleDateTime(year, month, day, 0,0,0));
//				//
//				// Temperatures
//				pEvent->SetTemp(atof(TokenArray[5])); // Mean temperature for 24 hour period
//				pEvent->SetMaxTemp(atof(TokenArray[5]));  // For now, Min and Max set to mean.  Need to determine if there's another waay
//				pEvent->SetMinTemp(atof(TokenArray[5]));
//				
//				// Rainfall
//				pEvent->SetRainfall(atof(TokenArray[3]) / 2.54);  // Convert from cm to inches
//				
//				// Decide if this is a foraging day.  This requires:
//				//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
//				//    Windspeed <= 8.94 m/s                AND
//				//    Rainfall <= .197 inches
//				//
//				// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
//				double windspeed = atof(TokenArray[6])/100;  // Convert from cm/s to m/s
//				pEvent->SetForage(
//					(pEvent->GetMaxTemp() > 12.0) && 
//					(windspeed <= GlobalOptions::Get().WindspeedThreshold()) &&
//					(pEvent->GetMaxTemp() <= 43.33) && 
//					(pEvent->GetRainfall() < GlobalOptions::Get().RainfallThreshold()));
//					
//				pEvent->SetForageInc(12.0, pEvent->GetMaxTemp(), pEvent->GetTemp());
//
//				double DayHours = CalcDaylightFromLatitude(Latitude, pEvent->m_Time.GetDayOfYear());
//				pEvent->SetDaylightHours(DayHours);  
//
//				pEvent->m_LineNum = LineNum++;
//				AddEvent(pEvent); 
//			}
//		}  
//
//	if (gl_RunGUI) ProgressDlg.DestroyWindow();
//	SetFileName(FileName);
//	HasBeenInitialized = true;
//	return true;
//
//}
//
//template<typename GridDataType>
//bool CWeatherEvents::LoadWeatherGridDataBinaryFile(CString FileName)
//{
//	// Clear the Weather Event List;
//	ClearAllEvents();
//
//	const bool computeHourlyTemperaturesEstimation = GlobalOptions::Get().ShouldComputeHourlyTemperatureEstimation();
//
//	WeatherGridData<GridDataType> data = WeatherGridDataNs::LoadGridData<GridDataType>(static_cast<const char*>(FileName));
//	auto& items = data.data();
//	if (items.size() > 0)
//	{
//		SetFileName(FileName);
//
//		// Get the latiture based on the filename 
//		double latitude = WeatherGridDataNs::GetLatitudeFromFilename(static_cast<const char*>(GetFileName()));
//
//		COleDateTime date = data.getStartTime();
//		COleDateTimeSpan oneDay(1, 0, 0, 0);
//
//		std::vector<CEvent*> events;
//		events.reserve(items.size());
//
//		// Create events using data from binary file
//		for (size_t itemIndex = 0; itemIndex < items.size(); itemIndex++)
//		{
//			auto& item = items[itemIndex];
//			DataItemAccessor<GridDataType> accessor(item);
//
//			CEvent* event = new CEvent();
//			event->SetLineNum(itemIndex);
//			event->SetTime(date);
//			if (!computeHourlyTemperaturesEstimation)
//			{
//				event->SetDaylightHours(WeatherGridDataNs::DayLength(latitude, WeatherGridDataNs::ComputeJDay(date)).daylength);
//			}
//			event->SetTemp((accessor.TMAX() + accessor.TMIN()) * 0.5);
//			event->SetMaxTemp(accessor.TMAX());
//			event->SetMinTemp(accessor.TMIN());
//
//			const double rainfall = accessor.PPT() * 0.0394;
//			event->SetRainfall(rainfall); // convert mm to inches
//
//			const double windSpeed = accessor.WIND();
//			UpdateForageAttributeForEvent(event, windSpeed);
//
//			AddEvent(event);
//			events.push_back(event);
//
//			date += oneDay;-
//		}
//
//		if (computeHourlyTemperaturesEstimation)
//		{
//			ComputeHourlyTemperatureEstimationAndUpdateForageInc(events);
//		}
//	}
//	HasBeenInitialized = true;
//	return true;
//}
//
//double CWeatherEvents::GetLatitudeFromFileName(CString WeatherFileName)
//{
//	// For EPA .wea files, the latitude is coded in the file name as follows:
//	// Filename = "xxxxx_grid_yy.yyy_lat.wea"  
//	// This function parses the filename and looks for the substring yy.yyy which is a valid 
//	// number and sets the latitude to this.  If a valid string is not found, the latitude is set to 30.
//	bool ValidLatitude = false;
//	double Latitude = 0;
//
//	int gloc = WeatherFileName.Find("_grid_");
//	int lloc = WeatherFileName.Find("_lat");
//	if ((gloc == -1) || (lloc == -1)) ValidLatitude = false; // unexpected file name format 
//	else
//	{
//		CString LatStg = WeatherFileName.Mid(gloc+6,6);
//		Latitude = atof(LatStg);
//		ValidLatitude = true;
//	}
//	if (ValidLatitude) return Latitude;
//	else return 30.0;
//}
//
//double CWeatherEvents::GetLatitudeFromFile(CString WeatherFileName)
//{
//	double Lat = 30;
//	int Direction = 1; // Either 1 or -1 for North or South latitude
//	CString Line;
//	CString LatStg = "Station Latitude:";
//	// Change filename extension from .dxf to .txt
//	WeatherFileName.MakeUpper();
//	WeatherFileName.Replace(".DVF",".TXT");
//	try
//	{
//		CStdioFile InputFile(WeatherFileName,CFile::shareDenyNone|CFile::modeRead);
//		bool Found =  false;
//		while (InputFile.ReadString(Line))
//		{
//			if (Line.Find(LatStg) >= 0)  // Found the line with the latitude
//			{
//				int CurPos = 0;
//				Line = Line.Right(Line.GetLength() - LatStg.GetLength());
//				CString Dirstg = Line.Tokenize(" ", CurPos);
//				if (Dirstg == "N") Direction = 1; else Direction = -1;
//				CString LDegStg = Line.Tokenize(" ", CurPos);
//				CString tempstg = Line.Tokenize(" ", CurPos);
//				CString LMinStg = Line.Tokenize(" ", CurPos);
//				Lat = (atof(LDegStg) + atof(LMinStg)/60) * Direction;
//				Found = true;
//				break;
//			}
//		}
//		if (!Found)
//		{
//			// File exists but Latitude line not found
//			Lat = 30;
//		}
//	}
//	catch(CFileException* e)
//	{
//		//File not found - use default latitude
//		Lat = 30;
//	}
//
//
//	return Lat;
//}
//
