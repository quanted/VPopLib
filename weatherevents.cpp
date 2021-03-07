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

// This struct is used to make sure that if a method gets called while calling another
// method, the behavior will be handled by the caller to avoid an infinite recursion
// Usage:
// <code>
//	sometype MyType::GetSomething() const
//	{
//	  sometype default = myDefaultValue;
//	  static call_if_not_calling caller;
//	  caller.call([this]{
//	    default = DoSomethingElse(); // soSomethingElse will call GetSomething at some point
//	  });
//    return default;
//	}
// </code>
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
	//  Returns the number of occurances of testchar in instg

	int count = 0;
	int len = instg.GetLength();

	for (int index=0; index < len; index++) if (instg[index]==testchar) count++;
	return count;
}

//// Compute the Forage and ForageInc attributes of a newly created event
//void UpdateForageAttributeForEvent(CEvent* event, double windSpeed)
//{
//	const bool forageDayBasedOnTemperatures = GlobalOptions::Get().ShouldForageDayElectionBasedOnTemperatures();
//	if (forageDayBasedOnTemperatures)
//	{
//		//
//		// Decide if this is a foraging day.  This requires:
//		//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
//		//    Windspeed <= 8.94 m/s                 AND
//		//    Rainfall <= .197 inches
//		//
//		// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
//		event->SetForage((event->GetMaxTemp() > 12.0) && (windSpeed <= GlobalOptions::Get().WindspeedThreshold()) &&
//			(event->GetMaxTemp() <= 43.33) && (event->GetRainfall() <= GlobalOptions::Get().RainfallThreshold()));
//	}
//	else
//	{
//		//
//		// Decide if this is a foraging day.  This requires:
//		//    Windspeed <= 8.94 m/s                AND
//		//    Rainfall <= .197 inches
//		//
//		// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
//		event->SetForage((windSpeed <= GlobalOptions::Get().WindspeedThreshold()) && (event->GetRainfall() <= GlobalOptions::Get().RainfallThreshold()));
//	}
//	// Here we set the Forage Increment using the default method, may be change later depending on execution options
//	event->SetForageInc(12.0, event->GetMaxTemp(), event->GetTemp());
//}


// Compute the Forage and ForageInc attributes of a newly created event
void CEvent::UpdateForageAttributeForEvent(double windSpeed)
{
	if (windSpeed == -1) windSpeed = GetWindspeed();  // For default, use the current windspeed
	const bool forageDayBasedOnTemperatures = GlobalOptions::Get().ShouldForageDayElectionBasedOnTemperatures();
	if (forageDayBasedOnTemperatures)
	{
		//
		// Decide if this is a foraging day.  This requires:
		//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
		//    Windspeed <= 8.94 m/s                 AND
		//    Rainfall <= .197 inches
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
		//    Rainfall <= .197 inches
		//
		// 5/21/2020: Changed the Windspeed from 21.13 meters/sec to 8.94 meters/sec
		SetForage((windSpeed <= GlobalOptions::Get().WindspeedThreshold()) && (GetRainfall() <= GlobalOptions::Get().RainfallThreshold()));
	}
	// Here we set the Forage Increment using the default method, may be change later depending on execution options
	SetForageInc(12.0, GetMaxTemp(), GetTemp());
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

void CEvent::UpdateForageDayState()
{
	// This function determines the value of m_ForageDay based on the values of the CEvent property.  Must be called for each CEvent
	//
	// Decide if this is a foraging day.  This requires:
	//    12.0 Deg C < MaxTemp < 43.33 Deg C    AND
	//    Windspeed <= 21.12 m/s                AND
	//    Rainfall <= .197 inches
	//
	if ((m_Windspeed > 20)	||
		(m_Temp <= -12.0)	||
		(m_Temp >= 43.33)	||
		//(m_Rainfall > 0.325)) m_ForageDay = false;  NOTE:  The value .325 cm was in here until 2021 when it was correctly converted from .197in to .500 cm
		(m_Rainfall > 0.500)) m_ForageDay = false;
	else m_ForageDay = true;
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

double CEvent::CalcDaylightFromLatitude(double Lat)
{
	// Reference:  Ecological Modeling, volume 80 (1995) pp. 87-95, called "A Model 
	// Comparison for Daylength as a Function of Latitude and Day of the Year."
	// Lat is in degrees - limited to be between +65 and -65 degrees.  Beyond that, Day is either 24 or 0 hours.
	// Probably not a lot of honeybee colonies inside the arctic and antarctic circles anyway.

	int DayNum = this->GetTime().GetDayOfYear();

	if (Lat < 0.0)
	{
		Lat = -Lat;
		DayNum = (DayNum + 182) % 365;
	}
	if (Lat > 65.0) Lat = 65.0;

	double PI = 3.14159265358979;
	double DaylightHours = 0;

	double P = asin(0.39795 * cos(0.2163108 + 2 * atan(0.9671396 * tan(0.00860 * (DayNum - 186)))));
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

//////////////////////////////////////////////////////////////////////
// CWeatherFile Member Functions
//////////////////////////////////////////////////////////////////////

//BOOL CWeatherFile::Open(LPCTSTR lpszFileName, 
//						UINT nOpenFlags, 
//						CFileException* pError)
//{
//	try
//	{
//		BOOL tf;
//		tf = CStdioFile::Open(lpszFileName, nOpenFlags, pError);
//		CFileStatus Status;
//		GetStatus(Status);
//		m_Size = (int)Status.m_size;
//		return tf;
//	}
//	catch (CFileException* e)
//	{
//		TCHAR stg[255];
//		e->GetErrorMessage(stg,255);
//		m_ErrorStg = CString("Error Opening File: ") + CString(stg);
//		return false;
//	}
//
//
//} 
//
//
//CWeatherFile::CWeatherFile()
//{
//	m_DateType = UNDEFINED;
//	m_CurrentLine = 0;
//	m_ErrorStg = "";
//	m_BytesRead = 0;
//}
//
//
//CWeatherFile::~CWeatherFile()
//{
//}
//
//
//BOOL CWeatherFile::GetLine(CString& stg)
//{
//	m_CurrentLine++;
//	BOOL pf = ReadString(stg);
//	if (pf) m_BytesRead += stg.GetLength() + 1;
//	return(pf);
//}
//
//
//bool CWeatherFile::IsHeaderPresent()
//{
//	CString stg;
//	GetLine(stg);
//	return (stg.Find("WEATHER_FILE") != -1);
//}
//
//void CWeatherFile::SeekToBegin()
//{
//	CStdioFile::SeekToBegin();
//	m_CurrentLine = 0;
//
//}
//
//CEvent* CWeatherFile::GetNextEvent()
//{
//	if (m_HeaderFormat == "MINMAX") return LineToEvent();
//	else return LinesToEvent();
//}

/* MODIFIED IN LIBRARY REFACTORING */
/*  This function is deleted since the library will not have any direct user interface */
//bool CWeatherFile::CreateHeader()
//{
//	/*  
//	Prompts the operator for some header information.  When this is read from the dialog 
//	box, the old file is renamed to ~<filename>, the header data is written to a new 
//	file with the proper file name, the old file contents are appended to the header, 
//	and the file is reopened.
//	*/
//#define TEMPFILENAME "Temp.wth"
//	CCreateWeatherHdr MyHeader;
//
//	if (MyHeader.DoModal()!=IDOK) return false;
//	else
//	{
//		try
//		{
//			CStringArray HeaderString;
//			MyHeader.GetHeaderStrings(HeaderString, this);
//
//			// Create temporary file and archive to build file with header
//			CStdioFile *tempFile = new CStdioFile(TEMPFILENAME,
//				CFile::modeWrite|CFile::modeCreate|CFile::shareDenyNone);
//
//			// Write header strings into new file
//			CString tempString1;
//			for (int i=0;i<HeaderString.GetSize();i++)
//			{
//				tempString1 = HeaderString.GetAt(i);
//				tempFile->WriteString(tempString1+"\n");
//			}
//
//			// Position the original file and archive to the beginning
//			SeekToBegin();
//
//			//  Read each line of the original file and write to temp
//			while (GetLine(tempString1))
//			{
//				tempFile->WriteString(tempString1+"\n");
//			}
//			ASSERT(tempFile);
//			delete tempFile;
//
//			// Rename original "filename"  to "~filename"
//			CString tempString2;
//			tempString1 = this->GetFileName();
//			tempString2 = CString("~") + tempString1;
//			this->Close();
//			CStdioFile::Rename(tempString1,tempString2);
//
//			// Rename new file to "filename"
//			CStdioFile::Rename(TEMPFILENAME,tempString1);
//
//			// Close and reopen new file and archive
//			this->Open(tempString1,CFile::modeRead|CFile::shareDenyNone);
//			SeekToBegin();
//			GetLine(tempString1);  // Position to second line - ready to go
//		}
//		catch (CFileException* e)
//		{
//			TCHAR stg[255];
//			e->GetErrorMessage(stg,255);
//			m_ErrorStg = CString("Error Creating Header: ") + CString(stg);
//			return false;
//		}
//		return true;
//	}
//}



//bool CWeatherFile::DigestHeader()
//{
//	//  Assumes the archive is pointed to the first line in the header.
//	//  This function will end with the archive pointed to the first
//	//  line past the line containing "----begin----".  Note that the headers
//	//  are different for MINMAX files and HOURLY files
//
//#define END_HEADER "----begin----"
//	try
//	{
//		CString theLine;
//		GetLine(theLine);
//		m_HeaderScale = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//		m_HeaderScale.TrimLeft();
//		m_HeaderScale.TrimRight();
//
//		GetLine(theLine);
//		m_HeaderFileName = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//		GetLine(theLine);
//		m_HeaderFormat = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//		m_HeaderFormat.TrimLeft();
//		m_HeaderFormat.TrimRight();
//
//		if (m_HeaderFormat == "MINMAX")
//		{
//
//			GetLine(theLine);
//			m_HeaderBeginTime = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//			GetLine(theLine);
//			m_HeaderBeginDate = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//			GetLine(theLine);
//			m_HeaderEndTime = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//			GetLine(theLine);
//			m_HeaderEndDate = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//			GetLine(theLine);
//			m_HeaderStartSim = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//
//			GetLine(theLine);
//			m_HeaderEndSim = theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1);
//		}
//
//		GetLine(theLine);
//		m_HeaderTempCol = 
//			atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//		if (m_HeaderFormat == "MINMAX")
//		{
//			GetLine(theLine);
//			m_HeaderMaxTempCol = 
//				atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//			GetLine(theLine);
//			m_HeaderMinTempCol = 
//				atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//		}
//			
//		GetLine(theLine);
//		m_HeaderSolarRadCol = 
//				atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//		GetLine(theLine);
//		m_HeaderWindspeedCol = 
//				atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//		GetLine(theLine);
//		m_HeaderRainCol = 
//			atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//		GetLine(theLine);
//		m_HeaderInterval = 
//			atoi(LPCTSTR(theLine.Right(theLine.GetLength() - theLine.ReverseFind('=') - 1)));
//
//		//  Now scan until the end of the header block is found
//		int end;
//		do
//		{
//			if (!GetLine(theLine))
//			{
//				m_ErrorStg = "Error Finding End of Header: ";
//				return false;
//			}
//			end = theLine.Find(END_HEADER);
//		}
//		while (end == -1);
//		return true;
//	}
//	catch (CFileException* e)
//	{
//		TCHAR stg[255];
//		e->GetErrorMessage(stg,255);
//		m_ErrorStg = CString("Error Reading Header: ") + CString(stg);
//		return false;
//	}
//}
//
//bool CWeatherFile::GetValidLine(COleDateTime& theTime, CString& theLine)
//{
//	//  Reads the file past any invalid lines.  Parses the first valid line, removing the 
//	//  date and time info and putting it in theTime.  Removes the date/time substring
//	//  from theLine and returns theLine.  This function also sets the member variable
//	//  m_PrevValidLinePos for use in backing up one line when processing HOURLY files.
//	//  NOTE:  CURRENTLY IMPLEMENTED FOR HOURLY FILES ONLY
//
//	bool ValidLine = false;
//
//	while (!ValidLine)
//	{
//		DWORD filepos = (DWORD)GetPosition();
//		if (!GetLine(theLine)) return false;  //  End of File Reached
//	
//		//  See if there are more than 15 blanks at the beginning of the line, if so
//		//  this is not a valid data line
//		int len = theLine.GetLength();
//		theLine.TrimLeft();
//		if (len - theLine.GetLength() > 15) continue;  // Invalid - Get Next Line
//
//		//  Assume date time format "ddddd tttttt" so find the position of the second
//		//  space character and extract the left to theDateTime
//		int spacecount = 0, index = 0;
//		while ((spacecount<2) && (index<theLine.GetLength()))
//		{
//			if (theLine[index++] == ' ') spacecount++;
//		}
//		if (spacecount < 2) continue;  //  Something wrong - invalid date
//		// Otherwise, "index" is the length of the datetime substring
//		
//
//
//		//  Now look for a valid date/time in the first "index" chars of the trimmed line
//		CString theDateTime = theLine.Left(index);  
//		theLine = theLine.Right(theLine.GetLength()-index);
//		int slashcount;
//		if (slashcount = CountChars(theDateTime,'/') == 0) continue;  // Invalid Line
//		//  Else assume valid date time
//		ValidLine = true;
//		m_PrevValidLinePos = filepos;
//
//
//		if (slashcount == 1)  // add "1999"
//		{
//			CString theDate = theDateTime.SpanExcluding(" ");
//			CString theTime = theDateTime.Right(theDateTime.GetLength() - theDate.GetLength());
//			theDate.TrimRight();
//			theDate += "1999 ";
//			theDateTime = theDate+theTime;
//		}
//		TRACE("GetValidLine - The Date Stg = ***%s***\n",theDateTime);
//		COleDateTime ODateTime;
//		ODateTime.ParseDateTime(LPCTSTR(theDateTime));
//
//		theTime = COleDateTime(ODateTime.GetYear(),
//						ODateTime.GetMonth(),
//						ODateTime.GetDay(),
//						ODateTime.GetHour(),
//						ODateTime.GetMinute(),0);
//	}
//	return true;
//
//}
//
//
//bool CWeatherFile::IsLineValid(CString theLine)
//{
//	//  See if there are more than 15 blanks at the beginning of the line, if so
//	//  this is not a valid data line
//	int len = theLine.GetLength();
//	theLine.TrimLeft();
//	if (len - theLine.GetLength() > 15) return false;
//
//	//  Now look for a valid date in the first 10 chars of the trimmed line
//	CString theDate = theLine.Left(10);  // extracts either mm/dd or ddd formats
//	int slashpos = theDate.Find('/');
//	if (slashpos == -1)  
//	//  A slash was not found, look for either doy or monthname format
//	{
//		// Get first substring
//		CString substr = theDate.SpanExcluding(" ");
//		if (IsMonth(substr)) //  The first substring in the line is a month name
//		{
//			m_DateType = MONTHNAME;
//			return true;
//		}
//		else						// Look for a day of year
//		{
//			int doy = atoi(LPCTSTR(substr));
//			if ((doy >0) && (doy<367)) 
//			{
//				m_DateType = DOY;
//				return true;
//			}
//			else return false;
//		}
//	}
//	else
//	//  Expecting mm/dd format or mm/dd/yy
//	{
//		int slashpos2 = theDate.ReverseFind('/');
//		int month = atoi(theDate.SpanExcluding("/"));
//		CString substr;
//		if (slashpos == slashpos2) substr = theDate.Mid(slashpos+1);  // mm/dd
//		else substr = theDate.Mid(slashpos+1, slashpos2-slashpos -1); // mm/dd/yy
//		int day = atoi(substr);
//		int year = 0;
//		if (slashpos2!=slashpos)
//		{
//			//year = atoi(theDate.Mid(slashpos2 + 1));
//			CString StgYear = theDate.Right(theDate.GetLength() - slashpos2 - 1);
//			year = atoi(StgYear);
//		}
//
//		if ((month > 0) && (month < 13) && (day > 0) && (day < 32)) 
//		{
//			if ((year == 0)&&(theDate.Mid(slashpos2 + 1).Find("00") >=0 )) m_DateType = MMDDYY;  // Year is 2-digit string "00"
//			else if (year > 0)
//			{
//				if (year < 100) m_DateType = MMDDYY; // A 2-digit year.  Make some assumptions about which century later
//				else m_DateType = MMDDYYYY; // A 4-digit year.
//			}
//			else m_DateType = MMDD;
//			return true;
//		}
//		else return false;
//	}
//}
//
//
//
//bool CWeatherFile::IsMonth(CString monthname)
//{
//	if (MonthToNum(monthname) == -1) return false;
//	else return true;
//
//}
//
//
//int CWeatherFile::MonthToNum(CString monthname)
//{
//	//  Returns the number of the month (i.e. April = 4) if monthname
//	//  is a valid name.  Otherwise returns -1.
//
//	if (monthname.GetLength()==0) return -1;
//
//	CString MonthNames[12] = {	"JANUARY", "FEBRUARY", "MARCH", "APRIL",
//								"MAY", "JUNE", "JULY", "AUGUST", 
//								"SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
//
//	CString MonthAbbrev[12] = {	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
//								"JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
//
//	bool found = false;
//	monthname.MakeUpper();
//	int i;
//	for (i = 0; i<12 ;i++) 
//	{
//		if ((MonthNames[i].Find(monthname)!=-1)||(MonthAbbrev[i].Find(monthname)!=-1)) 
//		{
//			found = true;
//			break;
//		}
//	}
//	if (found) return i+1;
//	else return -1;
//}
//
//
//
//CEvent* CWeatherFile::LinesToEvent()
//// Note: This is used for Hourly files.  Since we aren't using them,
//// this has not been updated since mid 1999.
//{
//	COleDateTime tempTime, ThisEventTime;
//	CString tempStg;
//	char TokenStgArray[32][255];
//	char InStg[255];
//	char* token;
//	char delims[] = " ";
//
//	float totalRain = 0, ThisEventRain = 0, DaytimeRain = 0;
//	float totalTemp = 0, ThisEventTemp = 0;
//	float ThisEventWind = 0;
//	float dayhours = 0;
//	bool forage = true;  // assume foraging day, negate if conditions ever fail
//
//	int count = 0;
//	do 
//	{
//		if (!GetValidLine(tempTime, tempStg)) 	// End of File reached during read
//		{
//			if (count == 0) return NULL;        // No valid Event lines read
//			else break;                         // Some lines read, go finish event
//		}
//
//		if (count == 0)	ThisEventTime = tempTime;  // First time thru
//	
//		// Check to see if we have just scanned a line representing a new day
//		// If so, this is where we leave the loop
//		if (ThisEventTime.GetDay()!=tempTime.GetDay())
//		{
//			GoToPrevLine();
//			break;
//		}
//
//		count++;
//
//		// Extract the tokens from the input line into TokenStgArray
//		strcpy_s(InStg,LPCTSTR(tempStg));
//		char * NextToken;
//		token = strtok_s(InStg, delims, &NextToken);
//		if (token != NULL) strcpy_s(TokenStgArray[0],token);
//		int i = 1;
//		while ((token != NULL) && (i<32))
//		{
//			token = strtok_s(NULL,delims,  &NextToken);
//			if (token != NULL) strcpy_s(TokenStgArray[i],token);
//			i++;
//		}
//
//		ThisEventRain = (float)atof(TokenStgArray[m_HeaderRainCol-2]);
//		ThisEventTemp = (float)atof(TokenStgArray[m_HeaderTempCol-2]);
//		ThisEventWind = (float)atof(TokenStgArray[m_HeaderWindspeedCol-2]);
//
//		// Convert temp to degC if necessary
//		if (m_HeaderScale == "F") ThisEventTemp = float(ThisEventTemp/float(1.8) - 17.778);
//
//		if (atof(TokenStgArray[m_HeaderSolarRadCol-2]) > 0) // Daytime
//		{
//			dayhours += 1;
//			DaytimeRain += ThisEventRain;
//			//  Is this still a foraging day?
//			if ((ThisEventWind > 20) || (ThisEventTemp <= 12.0) ||
//				(ThisEventTemp >= 43.33) || (DaytimeRain > 0.325)) forage = false;
//		}
//
//		totalTemp += ThisEventTemp;
//		totalRain += ThisEventRain;
//	}  while (true); //  End do loop.
//	//  Now build the event
//	
//	CEvent* tempEvent = new CEvent;
//
//
//	tempEvent->SetTime(ThisEventTime);
//	tempEvent->SetTemp(totalTemp/(float)count);
//	tempEvent->SetRainfall(totalRain);
//	tempEvent->SetDaylightHours(dayhours*m_HeaderInterval/60);	// m_HeaderInterval is
//	tempEvent->SetForage(forage);								// either 30 or 60
//
//	return tempEvent;
//
//}
//
//
//
//CEvent* CWeatherFile::LineToEvent()
//{
//
//	//  Define an array of strings to hold the tokens from each field.
//	//  Assume maximum number of columns (fields) = 32 and max field length = 255
//
//	CString theLine;
//	if (!GetLine(theLine)) return NULL;  // End of File reached
//
//	while (!IsLineValid(theLine))
//	{
//		if (!GetLine(theLine)) return NULL;
//	}
//
//	char TokenStgArray[32][255];
//
//	char InStg[255];
//	char* token;
//	char delims[] = " /\t";
//	strcpy_s(InStg,LPCTSTR(theLine));
//
//	// Extract the tokens from the input line
//	char * NextToken;
//	token = strtok_s(InStg, delims, &NextToken);
//	if (token != NULL) strcpy_s(TokenStgArray[0],token);
//	int i = 1;
//	while ((token != NULL) && (i<32))
//	{
//		token = strtok_s(NULL,delims, &NextToken);
//		if (token != NULL) strcpy_s(TokenStgArray[i],token);
//		i++;
//	}
//
//	/*  Check to see if this is a mm/dd, doy, or monthname format and set Offset accordingly.
//		The offset is used to map the file column number
//		to the index into the TokenStgArray[].  The following relationship applies 
//
//					#Columns							offset
//					Per Header			#tokens			value
//
//			DOY			1					1			  1
//			MMDD		1					2			  0
//			MMDDYY		1					3			 -1
//			MONTHNAME	2					2			  1
//
//
//	*/
//
//	int Offset;
//	theLine.TrimLeft();
//	if (m_DateType == MMDD) Offset = 0;
//	else if (m_DateType == MMDDYY) Offset = -1;
//	else if (m_DateType == MMDDYYYY) Offset = -1;
//	else if (m_DateType == MONTHNAME) Offset = 1;
//	else Offset = 1;  //  doy
//
//	CEvent* tempEvent = new CEvent;
//
//
//	//  Set the values of the temperatures and the rainfall
//	float maxtemp=(float)atof(TokenStgArray[m_HeaderMaxTempCol-Offset]);
//	float mintemp=(float)atof(TokenStgArray[m_HeaderMinTempCol-Offset]);
//	float windspeed = (float)atof(TokenStgArray[m_HeaderWindspeedCol-Offset]);
//
//	// Convert temp to degC if necessary
//	if (m_HeaderScale == "F") 
//	{
//		maxtemp = float(maxtemp/float(1.8) - 17.778);
//		mintemp = float(mintemp/float(1.8) - 17.778);
//	}
//
//	tempEvent->SetMaxTemp(maxtemp);
//	tempEvent->SetMinTemp(mintemp);
//
//	const double rainfall = atof(TokenStgArray[m_HeaderRainCol - Offset]) * 0.0394;
//	tempEvent->SetRainfall(rainfall); // convert mm to inches
//
//	tempEvent->SetLineNum(m_CurrentLine);
//	tempEvent->SetDaylightHours((float)atof(TokenStgArray[m_HeaderSolarRadCol-Offset]));
//	tempEvent->SetTemp((float)(maxtemp+mintemp)/2);
//
//	UpdateForageAttributeForEvent(tempEvent, windspeed);
//
//	//  The m_Time attribute is a little more complicated.  
//	if (m_DateType == DOY) //Day of Year Format
//	{
//		COleDateTimeSpan ts(atol(TokenStgArray[0]),0,0,0);
//		COleDateTime YearStart(COleDateTime::GetCurrentTime().GetYear(),1,1,0,0,0);
//		tempEvent->SetTime(YearStart + ts);
//	}
//	else if (m_DateType == MMDD)
//	{
//		COleDateTime tempTime(COleDateTime::GetCurrentTime().GetYear(),
//			atoi(TokenStgArray[0]),
//			atoi(TokenStgArray[1]),0,0,0);
//		tempEvent->SetTime(tempTime);
//	}
//	else if (m_DateType == MMDDYY)
//	{
//		int yr = atoi(TokenStgArray[2]);
//		(yr < 50) ? yr += 2000 : yr+=1900;
//		COleDateTime tempTime(yr,
//			atoi(TokenStgArray[0]),
//			atoi(TokenStgArray[1]),0,0,0);
//		tempEvent->SetTime(tempTime);
//	}
//	else if (m_DateType == MMDDYYYY)
//	{
//		CString stg = TokenStgArray[2];
//		int yr = atoi(TokenStgArray[2]);
//		COleDateTime tempTime(yr,
//			atoi(TokenStgArray[0]),
//			atoi(TokenStgArray[1]), 0, 0, 0);
//		tempEvent->SetTime(tempTime);
//	}
//	else  // Month name, day
//	{
//		ASSERT(IsMonth(TokenStgArray[0]));
//		COleDateTime tempTime(COleDateTime::GetCurrentTime().GetYear(),
//			MonthToNum(TokenStgArray[0]),
//			atoi(TokenStgArray[1]),0,0,0);
//		tempEvent->SetTime(tempTime);
//
//	}
//
//	return tempEvent;
//}
//
//
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
	CEvent* pEvent = m_EventList.GetAt(pos);
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
//			date += oneDay;
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

//double CWeatherEvents::CalcDaylightFromLatitude(double Lat, int DayNum)
//{
//	// Reference:  Ecological Modeling, volume 80 (1995) pp. 87-95, called "A Model 
//	// Comparison for Daylength as a Function of Latitude and Day of the Year."
//	// Lat is in degrees - limited to be between +65 and -65 degrees.  Beyond that, Day is either 24 or 0 hours.
//	// Probably not a lot of honeybee colonies inside the arctic and antarctic circles anyway.
//
//	if (Lat < 0.0)
//	{
//		Lat = - Lat;
//		DayNum = (DayNum + 182)%365;
//	}
//	if (Lat > 65.0) Lat = 65.0;
//
//	double PI = 3.14159265358979;
//	double DaylightHours = 0;
//
//	double P = asin(0.39795*cos(0.2163108 + 2*atan(0.9671396*tan(0.00860*(DayNum-186)))));
//	DaylightHours =
//		24 - (24/PI)*acos((sin(0.833*PI/180) + sin(Lat*PI/180)*sin(P))/cos(Lat*PI/180)*cos(P));
//	return DaylightHours;
//}

// TODO:  Will have to implement this in the library but not when reading the weather file
//void CWeatherEvents::ComputeHourlyTemperatureEstimationAndUpdateForageInc(std::vector<CEvent*>& events)
//{
//	try
//	{
//		const size_t eventsCount = events.size();
//
//		// Get the latitude based on the filename 
//		double latitude = WeatherGridDataNs::GetLatitudeFromFilename((const char*)GetFileName());
//
//		// Compute all daylengths as a first step to keep the solstice
//		std::vector<WeatherGridDataNs::DayLengthResult> dayLengths;
//		dayLengths.reserve(eventsCount);
//
//		float solstice = 0.0;
//
//		for (auto it = events.begin(); it != events.end(); it++)
//		{
//			const auto dayLength = WeatherGridDataNs::DayLength(latitude, WeatherGridDataNs::ComputeJDay((*it)->GetTime()));
//			dayLengths.push_back(dayLength);
//			solstice = (std::max)(dayLength.daylength, solstice);
//		}
//
//		CEvent* prevEvent = nullptr;
//		CEvent* event = nullptr;
//		for(size_t eventIndex=0; eventIndex<eventsCount; eventIndex++)
//		{	
//			CEvent* event = events[eventIndex];
//			const size_t nextIndex = eventIndex + 1;
//			CEvent* nextEvent = nextIndex < eventsCount? events[nextIndex] : nullptr;
//
//			// alter ForageInc taking into account estimated hourly temperature
//
//			WeatherGridDataNs::HourlyTempraturesEstimator estimator;
//
//			estimator.tmin = event->GetMinTemp();
//			estimator.tmax = event->GetMaxTemp();
//
//			auto& dayData = dayLengths[eventIndex];
//
//			estimator.sunrise = dayData.sunrise;
//			estimator.sunset = dayData.sunset;
//			estimator.daylength = dayData.daylength;
//
//			if (prevEvent != nullptr)
//			{
//				auto& prevDayData = dayLengths[eventIndex - 1];
//
//				estimator.prev_tmin = prevEvent->GetMinTemp();
//				estimator.prev_tmax = prevEvent->GetMaxTemp();
//				estimator.prev_sunset = prevDayData.sunset;
//			}
//
//			if (nextEvent != nullptr)
//			{
//				auto& nextDayData = dayLengths[eventIndex + 1];
//
//				estimator.next_tmin = nextEvent->GetMinTemp();
//				estimator.next_sunrise = nextDayData.sunrise;
//			}
//
//			estimator.compute();
//
//			int flighingDayLightHours = estimator.count_dayligth();
//
//			event->SetDaylightHours(estimator.daylength);
//			event->SetForageInc((std::min)(static_cast<float>(flighingDayLightHours), dayData.daylength) / solstice);
//
//			prevEvent = event;
//			event = nextEvent;
//		}
//	}
//	catch (std::exception & e)
//	{
//		// This exception only impact the computation of the adjusted Forage Increment, 
//		// simulation is able to continue anyway
//		CString stg = "Cannot extract latitude from filename: ";
//		//MyMessageBox(stg + GetFileName());
//	}
//}

//void CWeatherEvents::Serialize(CArchive& ar)
//{
//
//	if (ar.IsStoring())
//	{
//		// TODO: add storing code here
//	}
//	else
//	{
//		// TODO: add loading code here
//	}
//}
//


