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
	pEvent->UpdateForageDayState();


	// Need to Set Forage Day and ForageInc too
}


std::string  CString2StdString(CString cstr)
{
	std::string retstg((LPCTSTR)cstr);
	return retstg;
}

CString StdString2CString(std::string sstr)
{
	CString cstr(sstr.c_str());
	return cstr;
}

vector<string> CStringList2StringVector(CStringList& CSList)
{
	vector<string> StringVector;

	for (POSITION pos = CSList.GetHeadPosition(); pos != NULL;)
	{
		StringVector.push_back(CString2StdString(CSList.GetNext(pos)));
	}
	return StringVector;
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
		pColony->Create();  
		//pColony->InitializeColony();
		theSession.ClearErrorList();
		theSession.ClearInfoList();
		theSession.SetLatitude(30.0);
		theSession.AddToInfoList("Model Initialized");
		return true;
	}

	bool vplib::ClearResultsBuffers()
	{
		theSession.m_ResultsText.RemoveAll();
		theSession.AddToInfoList("Results Buffer Cleared");
		return true;
	}
	
	bool vplib::SetICVariables(std::string Name, std::string Value)
	{
		bool RetVal = false;
		CString info;
		CString CSName(Name.c_str());
		CString CSValue(Value.c_str());
		if (theSession.UpdateColonyParameters(Name, Value))
		{
			info.Format("Setting Variables.   Name = %s  Value = %s", CSName, CSValue);
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
	
	bool vplib::SetICVariables(vector<string>& NVPairs)
	{
		std::string stgName;
		std::string stgValue;
		for (size_t i = 0; i < NVPairs.size(); i++)
		{
			size_t eqpos = NVPairs[i].find("=");
			if (eqpos > 0)      // An equals sign must be present with at least one character preceding it
			{
				stgName = NVPairs[i].substr(0, eqpos);
				stgValue = NVPairs[i].substr(eqpos + 1, NVPairs[i].npos - (eqpos + 1));
				vplib::SetICVariables(stgName, stgValue);
			}
		}
		return true;
	}

	bool vplib::SetWeather(std::string WeatherEventString)
	{
		CString WECString = StdString2CString(WeatherEventString);
		CWeatherEvents* pWeatherEvents = theSession.GetWeather();
		CEvent* pEvent = new (CEvent);
		WeatherStringToEvent(WECString, pEvent);  // Consider adding a format ID in the parameter list to allow processing of different weather file formats
		pEvent->SetDaylightHours(pWeatherEvents->CalcDaylightFromLatitude(30.0, pEvent->GetTime().GetDayOfYear()));  // NOTE:  Generalize - set (or default) latitude somewhere else.
		pEvent->UpdateForageAttributeForEvent(pEvent->GetWindspeed());
		pWeatherEvents->AddEvent(pEvent);
		return true;
	}

	bool vplib::SetWeather(vector<string>& WeatherEventStringList)
	{
		for (size_t i = 0; i < WeatherEventStringList.size(); i++)
		{
			vplib::SetWeather(WeatherEventStringList[i]);
			theSession.AddToInfoList("Adding weather Event: " + WeatherEventStringList[i]);
		}
		// For inital testing, assume weather is fully loaded after this string vector is loaded
		theSession.SetSimStart(theSession.GetWeather()->GetBeginningTime());
		theSession.SetSimEnd(theSession.GetWeather()->GetEndingTime());
		theSession.GetWeather()->SetInitialized(true);  // Will have to work on this to capture the full scope of weather processing and error checking

		return true;
	}

	bool vplib::ClearWeather()
	{
		theSession.GetWeather()->ClearAllEvents();

		return true;
	}

	bool vplib::GetErrorList(vector<string>& ErrList)
	{
		ErrList.clear();
		for (POSITION pos = theSession.GetErrorList()->GetHeadPosition(); pos != NULL;)
		{
			ErrList.push_back(CString2StdString(theSession.GetErrorList()->GetNext(pos)));
		}
		return true;
	}

	bool vplib::GetInfoList(vector<string>& InfoList)
	{
		InfoList.clear();
		for (POSITION pos = theSession.GetInfoList()->GetHeadPosition(); pos != NULL;)
		{
			InfoList.push_back(CString2StdString(theSession.GetInfoList()->GetNext(pos)));
		}

		return true;
	}

	bool vplib::RunSimulation()
	{

		theSession.InitializeSimulation();
		theSession.Simulate();
		return true;
	}

	bool vplib::GetResults(vector<string>& ResultList)
	{
		bool ResVal = false;
		if (!theSession.m_ResultsText.IsEmpty())
		{
			ResultList = CStringList2StringVector(theSession.m_ResultsText);
			ResVal = true;
		}
		return ResVal;
	}

