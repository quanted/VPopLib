// VPopLib.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <string>
#include <vector>

// NOTE:  I would like to create this library so this file is the only header that needs to be imported by the using program.  
// I have embedded the contents of libvpop_Export.h, a system generated file, in this.  The primary purpose of this block of defined
// is to properly set the export macro "libvpop_EXPORT" to the right value based on Windows or Linux.  
// If a problem shows up will have to just include the livbpop_Export.h
//
//#include "libvpop_Export.h"
//#include "coblist.h"  
//#ifdef WIN32

#ifndef libvpop_EXPORT_H
#define libvpop_EXPORT_H

#ifdef LIBVPOP_STATIC_DEFINE
#  define libvpop_EXPORT
#  define LIBVPOP_NO_EXPORT
#else
#ifdef WIN32
#  ifndef libvpop_EXPORT
#    ifdef libvpop_EXPORTS
		/* We are building this library */
#      define libvpop_EXPORT __declspec(dllexport)
#    else
		/* We are using this library */
#      define libvpop_EXPORT __declspec(dllimport)
#    endif
#  endif
#else
#	ifndef libvpop_EXPORT
#		define libvpop_EXPORT
#	endif
#endif

#  ifndef LIBVPOP_NO_EXPORT
#    define LIBVPOP_NO_EXPORT 
#  endif
#endif

#ifndef LIBVPOP_DEPRECATED
#  define LIBVPOP_DEPRECATED __declspec(deprecated)
#endif

#ifndef LIBVPOP_DEPRECATED_EXPORT
#  define LIBVPOP_DEPRECATED_EXPORT libvpop_EXPORT LIBVPOP_DEPRECATED
#endif

#ifndef LIBVPOP_DEPRECATED_NO_EXPORT
#  define LIBVPOP_DEPRECATED_NO_EXPORT LIBVPOP_NO_EXPORT LIBVPOP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBVPOP_NO_DEPRECATED
#    define LIBVPOP_NO_DEPRECATED
#  endif
#endif

#endif /* libvpop_EXPORT_H */

//#endif


using namespace std;

namespace vplib // This is the interface for VPopLib
{
	bool libvpop_EXPORT InitializeModel();
	bool libvpop_EXPORT ClearResultsBuffers();
	bool libvpop_EXPORT SetICVariables(string Name, string Value);
	bool libvpop_EXPORT SetICVariables(vector<string>& NVPairs);
	bool libvpop_EXPORT SetWeather(string WeatherEventString);
	bool libvpop_EXPORT SetWeather(vector<string>& WeatherEventStringList);
	bool libvpop_EXPORT ClearWeather();
	bool libvpop_EXPORT GetErrorList(vector<string>& ErrList);
	bool libvpop_EXPORT GetInfoList(vector<string>& InfoList);
	bool libvpop_EXPORT RunSimulation();
	bool libvpop_EXPORT GetResults(vector<string>& ResultsList);
}