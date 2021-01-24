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
bool WeatherStringToEvent(CString theString, CEvent* pEvent, bool CalcDaylightByLat = false)
{
	//NOTE: Need to validate these values to ensure 
	/////////////////////////////////////////////////////////////////////////////
	//  The weather string format is a CString with comma or space delimited field as follows:
	//		theDate - any date format that is properly parsed by COleDateTime
	//		Maximum Temp - degrees C
	//		Minimum Temp - degrees C
	//		Mean Temp - degrees C
	//		Windspeed - m/s
	//		Daily Rainfall - mm
	//		Daylight Hours - number of hours of daylight

	int StrPos = 0;
	CString theToken;
	int InitStgLen = theString.GetLength();

	theToken = theString.Tokenize(" ,", StrPos);	//Date
	COleDateTime theDate;
	bool retval = theDate.ParseDateTime(theToken, VAR_DATEVALUEONLY);
	if (retval) // the date parse was successful.  Assume all the atof conversions are possible
	{
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
		if (CalcDaylightByLat)
		{
			pEvent->SetDaylightHours(pEvent->CalcDaylightFromLatitude(theSession.GetLatitude()));
		}
		else
		{
			theToken = theString.Tokenize(" ,", StrPos);	//Daylight hours
			pEvent->SetDaylightHours(atof(theToken));
		}
		pEvent->UpdateForageDayState();
	}
	return retval;
}
bool WeatherStringToEventWEA(CString theString, CEvent* pEvent, bool CalcDaylightByLat = false)
{
	//NOTE: Need to validate these values to ensure 
	/////////////////////////////////////////////////////////////////////////////
	//  The weather string format is a CString with comma or space delimited field as follows:
	//		theDate - any date format that is properly parsed by COleDateTime
	//		Maximum Temp - degrees C
	//		Minimum Temp - degrees C
	//		Mean Temp - degrees C
	//		Windspeed - m/s
	//		Daily Rainfall - mm
	//		Daylight Hours - number of hours of daylight

	/***********************************************************************************
		The EPA.wea Weather files are described as follows

		The weather files here were created per methods in Fry et al(2016); (see Fry_etal.pdf).
		Formatting is given in the supporting material(see SupportingMaterial_Fryetal.pdf).
		Briefly, the comma - separated fields are :
	mm, dd, yyyy, precipitation(cm / day), ETo(cm / day), mean daily temperature(C), wind speed(cm / s), solar radiation(La / day)

		The.wea filename format is xxxxx_grid_yy.yyy_lat.wea.The xxxxx string is an identifier which is used to map the weather station to
		the longitude of the station but we don't need that for this application.  The yy.yyy string is the weather station latitude (assumed to be N) which is
		used to calculate hours of sunlight per day.If a valid latitude is not found, a latitude of N 30 degrees is assumed.

		Reference
		Fry, M.M., Rothman, G., Young, D.F., and Thurman, N., 2016.  Daily gridded weather for exposure modeling, Environmental Modelling& Software, 82, 167 - 173, doi.org / 10.1016 / j.envsoft.2016.04.008

		*/

	int StrPos = 0;
	CString theToken;
	int InitStgLen = theString.GetLength();

	theToken = theString.Tokenize(" ,", StrPos);	//Date
	COleDateTime theDate;
	bool retval = theDate.ParseDateTime(theToken, VAR_DATEVALUEONLY);
	if (retval) // the date parse was successful.  Assume all the atof conversions are possible
	{
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
		if (CalcDaylightByLat)
		{
			pEvent->SetDaylightHours(pEvent->CalcDaylightFromLatitude(theSession.GetLatitude()));
		}
		else
		{
			theToken = theString.Tokenize(" ,", StrPos);	//Daylight hours
			pEvent->SetDaylightHours(atof(theToken));
		}
		pEvent->UpdateForageDayState();
	}
	return retval;
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
			//info.Format("Setting Variables.   Name = %s  Value = %s", CSName, CSValue);
			//theSession.AddToInfoList(info);
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
		bool retval = WeatherStringToEvent(WECString, pEvent, true);  // Consider adding a format ID in the parameter list to allow processing of different weather file formats
		if (retval)
		{
			pEvent->UpdateForageAttributeForEvent(pEvent->GetWindspeed());
			pEvent->UpdateForageDayState();
			pWeatherEvents->AddEvent(pEvent);
		}
		else
		{
			delete pEvent;
			theSession.AddToErrorList("Bad Weather File Record: " + WeatherEventString);
		}
		return retval;
	}


	// SetWeather with stringlist parameter - will return false if any of the weather events are bad - may be just one out of many.
	bool vplib::SetWeather(vector<string>& WeatherEventStringList)
	{
		bool retval = true;
		for (size_t i = 0; (retval) && (i < WeatherEventStringList.size()); i++)
		{
			string wthstr = WeatherEventStringList[i];
			retval = vplib::SetWeather(wthstr);
			if (retval) theSession.AddToInfoList("Adding weather Event: " + WeatherEventStringList[i]);
			//std::cout << "Adding weather Event: " << WeatherEventStringList[i] << std::endl;
		}
		// For inital testing, assume weather is fully loaded after this string vector is loaded
		if (theSession.GetWeather()->GetTotalEvents() > 0)
		{
			theSession.SetSimStart(theSession.GetWeather()->GetBeginningTime());
			theSession.SetSimEnd(theSession.GetWeather()->GetEndingTime());
			theSession.GetWeather()->SetInitialized(true);  // Will have to work on this to capture the full scope of weather processing and error checking
		}
		else retval = false;

		return retval;
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

