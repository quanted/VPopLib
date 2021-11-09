// WeatherEvents.h: interface for the CWeatherEvents class.
//
//////////////////////////////////////////////////////////////////////
/* MODIFIED IN LIBRARY REFACTORING 
	All file and UI iterfaces are removed with the CEvent and CWeatherEvents classes being 
	only local data and accessors.  The responsibility for loading the CWeatherEvents class is 
	outside this library
*/

#if !defined(AFX_WEATHEREVENTS_H__40D18204_C275_11D2_8918_9F75F84DB713__INCLUDED_)
#define AFX_WEATHEREVENTS_H__40D18204_C275_11D2_8918_9F75F84DB713__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "cstring.h"
#include "cstring.format.h"
#include "coledatetime.h"
#include "cptrlist.h"

#include <vector>
#include <string>

class CEvent : public CObject
{
	friend class CWeatherEvents;
private:
	COleDateTime m_Time;
	double m_Temp;		//  Average Temperature in degC
	double m_MaxTemp;	//  Maximum Temperature in degC
	double m_MinTemp;	//  Minumum Temperature in degC
	double m_Windspeed;	//  Windspeed in m/s
	double m_Rainfall;	//  Rainfall in inches
	bool m_ForageDay;
	double m_ForageInc;	//  Increment of a Forage Day
	double m_DaylightHours;
	int m_LineNum;
public:
	CEvent();
	~CEvent();
	CString ToString(); // Returns a human-readable text version of the event
	CEvent(CEvent& event);  // Copy Constructor
	COleDateTime GetTime() {return m_Time;}
	CString GetDateStg(CString format = "%d/%m/%Y");
	double GetTemp();
	double GetMaxTemp();
	double GetMinTemp();
	double GetRainfall() {return m_Rainfall;}
	double GetWindspeed() { return m_Windspeed; }
	//void UpdateForageDayState();
	void UpdateForageAttributeForEvent( double Latitude, double windSpeed = -1 );
	bool IsForageDay();
	bool IsWinterDay();
	int GetLineNum() {return m_LineNum;}
	double GetDaylightHours();
	void SetTime(COleDateTime time) {m_Time = time;}
	void SetTemp(double temp) {m_Temp = temp;}
	void SetMaxTemp(double maxTemp) {m_MaxTemp = maxTemp;}
	void SetMinTemp(double minTemp) {m_MinTemp = minTemp;}
	void SetRainfall(double rainfall) {m_Rainfall = rainfall;}
	void SetWindspeed(double windspeed) { m_Windspeed = windspeed; }
	void SetForage(bool Forage);
	void SetForageInc(double forageInc) {m_ForageInc = forageInc;};
	void SetForageInc(double TThresh, double TMax, double TAve);
	void SetHourlyForageInc(double Latitude);
	double GetForageInc();
	void SetLineNum(int line) {m_LineNum = line;}
	void SetDaylightHours(double hrs) {m_DaylightHours = hrs;}
	double CalcTodayDaylightFromLatitude(double Lat);
	double CalcDaylightFromLatitudeDOY(double Lat, int DayOfYear);
	double CalcFlightDaylight(double daylength, double MinTempThreshold = 12.0, double MaxTempThreshold = 43.0);

	//  Overloaded Operators
	CEvent operator = (CEvent& event);
	CEvent operator + (CEvent event);
	void operator += (CEvent event);


};


class CWeatherEvents : public CObject  
{
private:
	CString m_Filename;
	POSITION pos;
	CTypedPtrList<CPtrList, CEvent*> m_EventList;
	bool m_HasBeenInitialized;
	double m_Latitude;  



public:
	CWeatherEvents();
	virtual ~CWeatherEvents();
	void ClearAllEvents();
	bool AddEvent(CEvent* theEvent);
	bool RemoveCurrentEvent();
	COleDateTime GetBeginningTime(); // Returns the first date in the actual weather file
	COleDateTime GetEndingTime();    // Returns the last date in the actual weather file
	COleDateTime GetCurrentTime();
	double CalcDaylightFromLatitude(double Lat, int DayNum);
	int GetCurrentLineNumber();
	int GetTotalEvents();
	void SetFileName(CString fname) {m_Filename = fname;}
	int CheckInterval();
	int CheckSanity();

	//! Enable to load grid binary files directly check WeatherGridData.h to see supported formats
	//template<typename GridDataType>
//	bool LoadWeatherGridDataBinaryFile(CString FileName);

	//void ComputeHourlyTemperatureEstimationAndUpdateForageInc(std::vector<CEvent*>& events);  //TODO:  Will have to implement this differently in the library
	void SetLatitude(double lat) { m_Latitude = lat; }
	double GetLatitude() { return m_Latitude; }
	bool IsInitialized() {return m_HasBeenInitialized;}
	void SetInitialized(bool val) { m_HasBeenInitialized = val; }
	//  Access
	void GoToFirstEvent();
	CEvent* GetNextEvent();
	CEvent* GetDayEvent(COleDateTime theTime);
	CEvent* GetFirstEvent();

	// Operators
protected:
	//double GetLatitudeFromFile(CString WeatherFileName);
	//double GetLatitudeFromFileName(CString WeatherFileName);
};

//  Free Functions
int CountChars(CString instg, TCHAR testchar);

#endif // !defined(AFX_WEATHEREVENTS_H__40D18204_C275_11D2_8918_9F75F84DB713__INCLUDED_)
