// VPopLib.cpp : Defines the entry point for the application.
//

#include "VPopLib.h"
#include "Session.h"
#include "Colony.h"
#include "stdafx.h"
#include "WeatherEvents.h"

using namespace std;

// Define the simulation global variables and classes
CVarroaPopSession theSession;



// Define the local functions to VPopLib.  The interface implementations are at the bottom of this file

//**************************************************************************
// WeatherStringToEvent
//
/* The string format is
*	Date(MM/DD/YY)
*   Max Temp (C)
*   Min Temp (C)
*   Ave Temp (C)
*   Windspeed (m/s)
*   Rainfall amount (mm)
*   Hours of Daylight (hr)
*
* Space or comma delimited
*
*/
void WeatherStringToEvent(CString theString, CEvent* pEvent)
{
	//NOTE: Need to validate these values to ensure 
	int StrPos = 0;
	CString theToken;
	int InitStgLen = theString.GetLength();

	theToken = theString.Tokenize(" ,", StrPos);	//Date
	COleDateTime theDate;
	theDate.ParseDateTime(theToken, VAR_DATEVALUEONLY);
	pEvent->SetTime(theDate);
	theToken = theString.Tokenize(" ,", StrPos);	//Max Temp
	pEvent->SetMaxTemp(atof(theToken));
	theToken = theString.Tokenize(" ,", StrPos);	//Min Temp
	pEvent->SetMinTemp(atof(theToken));
	theToken = theString.Tokenize(" ,", StrPos);	//Ave Temp
	pEvent->SetTemp(atof(theToken));
	theToken = theString.Tokenize(" ,", StrPos);	//Windspeed
	pEvent->SetWindspeed(atof(theToken));
	theToken = theString.Tokenize(" ,", StrPos);	//Rainfall Amount
	pEvent->SetRainfall(atof(theToken));
	theToken = theString.Tokenize(" ,", StrPos);	//Daylight hours
	pEvent->SetDaylightHours(atof(theToken));


	// Need to Set Forage Day and ForageInc too
}







//*****************************************************************************************************************
//	This following functions provide the interface to VPopLib.  All using the vplib namespace
//
//
//
//*****************************************************************************************************************

	bool vplib::InitializeModel()
	{
		CColony* pColony = theSession.GetColony();
		pColony->InitializeColony();
		theSession.ClearErrorList();
		theSession.ClearInfoList();
		theSession.AddToInfoList("Colony Initialized");
		return true;
	}

	bool vplib::ClearResultsBuffers()
	{
		theSession.m_ResultsText.RemoveAll();
		theSession.AddToInfoList("Results Buffer Cleared");
		return true;
	}
	
	bool vplib::SetICVariables(CString Name, CString Value)
	{
		bool RetVal = false;
		CString info;
		if (theSession.UpdateColonyParameters(Name, Value))
		{
			info.Format("Setting Variables.   Name = %s  Value = %s", Name, Value);
			theSession.AddToInfoList(info);
			RetVal = true;
		}
		else
		{
			CString err;
			err.Format("Failed to set %s to %s", Name , Value);
			theSession.AddToErrorList(err);
			RetVal = false;
		}
		return RetVal;
	}
	
	bool vplib::SetICVariables(CStringList NVPairs)
	{
		CString NVPair;
		CString Name; 
		CString Value;
		for (POSITION pos = NVPairs.GetHeadPosition(); pos != NULL;)
		{
			NVPair = NVPairs.GetNext(pos);
			int eqpos = NVPair.Find('=');
			if (eqpos > 0)  // An equals sign must be present with at least one character preceding it
			{
				Name = NVPair.Left(eqpos);
				Value = NVPair.Right(NVPair.GetLength() - eqpos - 1);
				vplib::SetICVariables(Name, Value);
			}
		}
		return true;
	}

	bool vplib::SetWeather(CString WeatherEventString)
	{
		CWeatherEvents* pWeatherEvents = theSession.GetWeather();
		CEvent* pEvent = new (CEvent);
		WeatherStringToEvent(WeatherEventString, pEvent);  // Consider adding a format ID in the parameter list to allow processing of different weather file formats
		pWeatherEvents->AddEvent(pEvent);
		return true;
	}

	bool vplib::SetWeather(CStringList WeatherEventStringList)
	{
		int numDates = 0;
		POSITION pos;
		pos = WeatherEventStringList.GetHeadPosition();
		for (pos = WeatherEventStringList.GetHeadPosition(); pos != NULL;)
		{
			vplib::SetWeather(WeatherEventStringList.GetNext(pos));
		}
		return true;
	}

	bool vplib::ClearWeather()
	{
		theSession.GetWeather()->ClearAllEvents();

		return true;
	}

	bool vplib::GetErrorList(CStringList& ErrList)
	{
		ErrList.RemoveAll();
		for (POSITION pos = theSession.GetErrorList()->GetHeadPosition(); pos != NULL;)
		{
			ErrList.AddTail(theSession.GetErrorList()->GetNext(pos));
		}
		return true;
	}

	bool vplib::GetInfoList(CStringList& InfoList)
	{
		InfoList.RemoveAll();
		for (POSITION pos = theSession.GetInfoList()->GetHeadPosition(); pos != NULL;)
		{
			InfoList.AddTail(theSession.GetInfoList()->GetNext(pos));
		}

		return true;
	}

	bool vplib::RunSimulation()
	{
		return true;
	}

	bool vplib::GetResults(CStringList* ResultList)
	{
		bool ResVal = false;
		if (!theSession.m_ResultsText.IsEmpty())
		{
			ResultList = &theSession.m_ResultsText;
			ResVal = true;
		}
		return ResVal;
	}


//static int MyMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
//{
//	return -1;
//}
