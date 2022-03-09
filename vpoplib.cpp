// VPopLib.cpp : Defines the entry point for the application.
//

#include "vpoplib.h"
#include "session.h"
#include "colony.h"
#include "stdafx.h"
#include "weatherevents.h"

using namespace std;

// Initialize  the session 
CVarroaPopSession theSession;



// Define the local functions to VPopLib.  The interface implementations are at the bottom of this file

bool GetLibVersionCP(char* version, int bufsize)
{
	bool retval = false;
	string strversion = VPOPLIB_VERSION;
	if (strversion.size() + 1 <= bufsize)
	{
		size_t length = strversion.copy(version, strversion.size());
		version[length] = '\0';
		retval = true;
	}
	return retval;
}

bool GetLibVersion(string& version)
{
	version = VPOPLIB_VERSION;
	return true;
}



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
bool WeatherStringToEvent(CString theString, CEvent* pEvent, bool CalcDaylightByLat = true)
{
	
	/////////////////////////////////////////////////////////////////////////////
	//  The weather string format is a CString with comma or space delimited field as follows:
	//		theDate - any date format that is properly parsed by COleDateTime
	//		Maximum Temp - degrees C
	//		Minimum Temp - degrees C
	//		Mean Temp - degrees C
	//		Windspeed - m/s
	//		Daily Rainfall - mm
	//		Daylight Hours - number of hours of daylight
	//
	//	Note:  If CalcDaylightByLat = true, the last field of theString, Daylight Hours, need not be present and is ignored if present.

	int StrPos = 0;
	CString theToken;

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
			pEvent->SetDaylightHours(pEvent->CalcTodayDaylightFromLatitude(theSession.GetLatitude()));
		}
		else
		{
			theToken = theString.Tokenize(" ,", StrPos);	//Daylight hours
			pEvent->SetDaylightHours(atof(theToken));
		}
		pEvent->UpdateForageAttributeForEvent(theSession.GetLatitude(), pEvent->GetWindspeed());
	}
	return retval;
}

/*******************************************************************************
* Converts a Nutrient string into a SNCElement struct
* Nutrient String Format is:  Date, Nectar, Pollen
* 
*/
bool String2NutrientElement(SNCElement &theElement, CString theString)
{
	CString theToken;
	int StrPos = 0;
	theToken = theString.Tokenize(" ,", StrPos);
	bool retval = theElement.m_NCDate.ParseDateTime(theToken, VAR_DATEVALUEONLY);  // If date parses, assume other items are valid
	if (retval)
	{
		theToken = theString.Tokenize(" ,", StrPos);
		theElement.m_NCNectarCont = atof(theToken);
		theToken = theString.Tokenize(" ,", StrPos);
		theElement.m_NCPollenCont = atof(theToken);
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

char** StringVector2CharStringArray(vector<string> stringvector)
{
	// Note - the calling function is responsible for freeing the memory created with this
	int VectorLength = stringvector.size();
	char** CPArray = new char* [VectorLength]();
	for (int i = 0; i < VectorLength; i++)
	{
		CPArray[i] = new char[stringvector[i].size() + 1];
		strcpy(CPArray[i], stringvector[i].c_str());
	}

	return CPArray;
}





//*****************************************************************************************************************
//	This following functions provide the interface to VPopLib.  
//
//
//
//*****************************************************************************************************************

	bool InitializeModel()
	{
		CColony* pColony = theSession.GetColony();
		pColony->Create();  
		theSession.ClearErrorList();
		theSession.ClearInfoList();
		//theSession.AddToInfoList("Model Initialized");
		return true;
	}

	bool ClearResultsBuffer()
	{
		theSession.m_ResultsText.RemoveAll();
		//theSession.AddToInfoList("Results Buffer Cleared");
		return true;
	}

	bool SetLatitude(double Lat)
	{
		theSession.SetLatitude(Lat);
		return true;
	}


	bool SetICVariablesCP(char* NameCP, char* ValueCP)
	{
		string Name = NameCP;
		string Value = ValueCP;
		return (SetICVariablesS(Name, Value));
	}

	bool SetICVariablesCPA(char** NVPairsCPA, int Count, bool ResetICs)
	{
		vector<string> NVPairs(NVPairsCPA, NVPairsCPA + Count);
		// Need to ready the session to take on a brand new set of ICs.  That will
		// require the clearing of all DRVs.  The default parameter ResetICs is set to true.
		//
		if (ResetICs)  // Need to clear the date range value lists before each new load of ICs if ResetICs is true
		{
			theSession.GetColony()->m_InitCond.m_AdultLifespanDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_ForagerLifespanDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_EggTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_BroodTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_LarvaeTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_AdultTransitionDRV.ClearAll();
			theSession.GetColony()->m_MiteTreatmentInfo.ClearAll();
		}

		return SetICVariablesV(NVPairs);
	}

	bool SetICVariablesS(std::string Name, std::string Value)
	{
		bool RetVal = true;
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
	
	bool SetICVariablesV(vector<string>& NVPairs, bool ResetICs)
	{
		// Need to ready the session to take on a brand new set of ICs.  That will
		// require the clearing of all DRVs.  The default parameter ResetICs is set to true.
		//
		if (ResetICs)  
		{
			theSession.GetColony()->m_InitCond.m_AdultLifespanDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_ForagerLifespanDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_EggTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_BroodTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_LarvaeTransitionDRV.ClearAll();
			theSession.GetColony()->m_InitCond.m_AdultTransitionDRV.ClearAll();
			theSession.GetColony()->m_MiteTreatmentInfo.ClearAll();
		}

		std::string stgName;
		std::string stgValue;
		for (size_t i = 0; i < NVPairs.size(); i++)
		{
			size_t eqpos = NVPairs[i].find("=");
			if (eqpos > 0)      // An equals sign must be present with at least one character preceding it
			{
				stgName = NVPairs[i].substr(0, eqpos);
				stgValue = NVPairs[i].substr(eqpos + 1, NVPairs[i].npos - (eqpos + 1));
				SetICVariablesS(stgName, stgValue);
			}
		}
		return true;
	}

	bool SetWeatherCP(char* WeatherEventStringCP)
	{
		string WeatherEventString = WeatherEventStringCP;
		return (SetWeatherS(WeatherEventString));
	}

	bool SetWeatherCPA(char** WeatherEventStringListCPA, int Count)
	{
		vector<string> WeatherEventStringList(WeatherEventStringListCPA, WeatherEventStringListCPA + Count);
		bool retval = SetWeatherV(WeatherEventStringList);
		return retval;

	}

	bool SetWeatherS(std::string WeatherEventString)
	{
		CString WECString = StdString2CString(WeatherEventString);
		CWeatherEvents* pWeatherEvents = theSession.GetWeather();
		CEvent* pEvent = new (CEvent);
		bool retval = WeatherStringToEvent(WECString, pEvent, true);  // Consider adding a format ID in the parameter list to allow processing of different weather file formats
		if (retval)
		{
			pEvent->UpdateForageAttributeForEvent(theSession.GetLatitude(), pEvent->GetWindspeed());
			pWeatherEvents->AddEvent(pEvent);
		}
		else
		{
			delete pEvent;
			theSession.AddToErrorList("Bad Weather String Format: " + WeatherEventString);
		}
		return retval;
	}


	// SetWeather with stringlist parameter - will return false if any of the weather events are bad - may be just one out of many.
	bool SetWeatherV(vector<string>& WeatherEventStringList)
	{
		bool retval = true;
		for (size_t i = 0; (retval) && (i < WeatherEventStringList.size()); i++)
		{
			string wthstr = WeatherEventStringList[i];
			retval = SetWeatherS(wthstr);
			string outstring;

		}

		if (theSession.GetWeather()->GetTotalEvents() > 0)
		{
			theSession.SetSimStart(theSession.GetWeather()->GetBeginningTime());
			theSession.SetSimEnd(theSession.GetWeather()->GetEndingTime());
			theSession.GetWeather()->SetInitialized(true);  // Will have to work on this to capture the full scope of weather processing and error checking
		}
		else retval = false;

		return retval;
	}

	bool ClearWeather()
	{
		theSession.GetWeather()->ClearAllEvents();
		return true;
	}

	bool SetContaminationTable(vector<string>& ContaminationTableList)
	{
		// Contamination Table records are strings in comma-separated format of:  Date, NectarConcentration, PollenConcentration
		// Concentrations are in grams of active ingredient / total grams material.  Normally nanograms AI per grams nectar for example 
		// so would be a number like 0.000000020  or it's scientific notation equivalent 2.00E-8
		bool retval = false;
		if (ContaminationTableList.size() > 0)
		{
			CColony* pColony = theSession.GetColony();
			pColony->m_NutrientCT.RemoveAll();
			COleDateTime theTime;
			double theNecConc;
			double thePolConc;
			string CTRecord;
			SNCElement theElement;
			for (int i = 0; i < ContaminationTableList.size(); i++)
			{
				if (String2NutrientElement(theElement, ContaminationTableList[i]))
				{
					pColony->m_NutrientCT.AddContaminantConc(theElement);
					retval = true; // Set true if any items are added
				}
			}
		}
		return retval;
	}

	bool SetContaminationTableCPA(char** ContaminationTableListCPA, int Count)
	{
		vector<string> ContaminationTableList(ContaminationTableListCPA, ContaminationTableListCPA + Count);
		return SetContaminationTable(ContaminationTableList);
	}

	bool ClearContaminationTable()
	{
		theSession.GetColony()->m_NutrientCT.RemoveAll();
		return true;
	}

	bool GetErrorList(vector<string>& ErrList)
	{
		ErrList.clear();
		for (POSITION pos = theSession.GetErrorList()->GetHeadPosition(); pos != NULL;)
		{
			ErrList.push_back(CString2StdString(theSession.GetErrorList()->GetNext(pos)));
		}
		return true;
	}

	bool GetErrorListCPA(char*** ErrListCPA, int* pCount)
	{
		vector<string> ErrList;
		if (GetErrorList(ErrList))
		{
			*ErrListCPA = StringVector2CharStringArray(ErrList);
			*pCount = ErrList.size();
			return true;
		}
		else return false;
	}

	bool ClearErrorList()
	{
		theSession.GetErrorList()->RemoveAll();
		return true;
	}

	bool GetInfoList(vector<string>& InfoList)
	{
		if (theSession.GetInfoList()->GetCount() > 0)
		{
			InfoList.clear();
			for (POSITION pos = theSession.GetInfoList()->GetHeadPosition(); pos != NULL;)
			{
				string theString = CString2StdString(theSession.GetInfoList()->GetNext(pos));
				InfoList.push_back(theString);
			}
		}
		return true;
	}

	bool GetInfoListCPA(char*** InfoListCPA, int* pCount)
	{
		vector<string> InfoList;
		if (GetErrorList(InfoList))
		{
			*InfoListCPA = StringVector2CharStringArray(InfoList);
			*pCount = InfoList.size();
			return true;
		}
		else return false;
	}


	bool ClearInfoList()
	{
		theSession.GetInfoList()->RemoveAll();
		return true;
	}

	bool RunSimulation()
	{
		theSession.InitializeSimulation();
		theSession.Simulate();
		return true;
	}

	bool GetResultsCPA(char*** ResultsListCPA, int* pCount)
	{
		vector<string> ResultsList;
		if (GetResults(ResultsList))
		{
			*ResultsListCPA = StringVector2CharStringArray(ResultsList);
			*pCount = ResultsList.size();
			return true;
		}
		else return false;
	}



	bool GetResults(vector<string>& ResultList)
	{
		bool ResVal = false;
		if (!theSession.m_ResultsText.IsEmpty())
		{
			ResultList = CStringList2StringVector(theSession.m_ResultsText);
			ResVal = true;
		}
		return ResVal;
	}

