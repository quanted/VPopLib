// VPopLib.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

#include "stdafx.h"
#include "coblist.h"  




namespace vplib // This is the interface for VPopLib
{
	bool libvpop_EXPORT InitializeModel();
	bool libvpop_EXPORT ClearResultsBuffers();
	bool libvpop_EXPORT SetICVariables(CString Name, CString Value);
	bool libvpop_EXPORT SetICVariables(CStringList NVPairs);
	bool libvpop_EXPORT SetWeather(CString WeatherEventString);
	bool libvpop_EXPORT SetWeather(CStringList WeatherEventStringList);
	bool libvpop_EXPORT ClearWeather();
	bool libvpop_EXPORT GetErrorList(CStringList& ErrList);
	bool libvpop_EXPORT GetInfoList(CStringList& InfoList);
	bool libvpop_EXPORT RunSimulation();
	bool libvpop_EXPORT GetResults(CStringList* ResultList);

};

