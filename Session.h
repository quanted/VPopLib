#pragma once
//#include "Matrix.h"
#include "coblist.h"
#include "coledatetime.h"
#include "Mite.h"
#include "WeatherEvents.h"
#include "MiteTreatments.h"
#include "Colony.h"

//
// CVarroaPopSession contains the state of the simulation.  
//

class CVarroaPopSession
{
public:
	CVarroaPopSession();
	virtual ~CVarroaPopSession();

	

	//  Output/Plotting Attributes;
	CStringList m_ResultsHeader;// Header Strings for ListView Control
	CStringList m_ResultsFileHeader; // Header Strings for File
	CStringList m_ResultsText;  // Results of Simulation 
	CStringList m_ResultsFileText; // Results of Simulation for file storage
	bool m_DispWeeklyData;		// Defines whether numeric results are generated
								//    Weekly or Daily



	// Options Selection from COptions Dialog
	BOOL m_ColTitles;
	BOOL m_InitConds;
	BOOL m_Version;
	BOOL m_WeatherColony;
	int m_FieldDelimiter;
	int m_DispFrequency;



protected:
	CColony theColony;
	CWeatherEvents* m_pWeather;
	bool m_FirstResultEntry;
	CString m_DefaultPathName;
	CString m_WeatherFileName;
	bool m_WeatherLoaded;
	bool m_ShowWarnings;
	double m_Latitude;  // The latitude of the run.  Defaults to 30degrees N

	//  Errors, Status, etc
	CStringList m_ErrorList;		//m_ErrorList holds all errors generated in a simulation run.  Can be cleared
	CStringList m_InformationList;	//m_InformationList holds any commentary or info generated during a simulation run.  Does not include errors.

	// Simulation Data
	COleDateTime m_SimStartTime;
	COleDateTime m_SimEndTime;
	bool m_SimulationComplete;
	bool m_ResultsReady;

public:
	// Immigration Data
	CString m_ImmigrationType;
	CMite m_TotImmigratingMites;
	CMite m_IncImmigratingMites;
	CMite m_CumImmigratingMites;
	double m_ImmMitePctResistant;
	COleDateTime m_ImmigrationStartDate;
	COleDateTime m_ImmigrationEndDate;
	bool m_ImmigrationEnabled;

	// Re-Queening Data
	UINT	m_RQEggLayingDelay;
	double	m_RQWkrDrnRatio;
	BOOL	m_RQEnableReQueen;
	int		m_RQScheduled;
	double	m_RQQueenStrength;
	int		m_RQOnce;
	COleDateTime	m_RQReQueenDate;

	// Varroa Miticide Treatment Data;
	// NOTE:  Need to change this to a list to support multiple treatments
	CMiteTreatments m_MiteTreatments;

	COleDateTime	m_VTTreatmentStart;
	UINT	m_VTTreatmentDuration;
	UINT	m_VTMortality;
	double	m_InitMitePctResistant;
	BOOL	m_VTEnable;

	// Varroa Spore Treatment Data
	BOOL	m_SPEnable;
	COleDateTime	m_SPTreatmentStart;
	int		m_SPInitial;
	double	m_Mort10;
	double	m_Mort25;
	double	m_Mort50;
	double	m_Mort75;
	double	m_Mort90;

	// Comb Removal
	COleDateTime	m_CombRemoveDate;
	BOOL	m_CombRemoveEnable;
	double  m_CombRemovePct;

	// EPA Mortality
	//CIEDItem m_IEDItem;  //Note:  This variable should be removed but it will change the session file format
	BOOL m_IEDEnable;

	CString m_ResultsFileFormatStg;

protected:

	//CVarroaPopSessionBridge* m_Bridge;

	// Operations
public:

	//void SetBridge(CVarroaPopSessionBridge* bridge) { m_Bridge = bridge; }
	//CVarroaPopSessionBridge* GetBridge() const { return m_Bridge; }

	CWeatherEvents* GetWeather() { return m_pWeather; }
	CColony* GetColony() { return &theColony; }
	int GetDocumentLength();
	bool DateInRange(COleDateTime StartRange, COleDateTime StopRange, COleDateTime theTime);
	bool CheckDateConsistency(bool ShowWarning);
	bool IsWeatherLoaded() { return m_WeatherLoaded; }
	bool IsShowWarnings() { return m_ShowWarnings; }
	void SetShowWarnings(bool Warn) { m_ShowWarnings = Warn; }
	void SetWeatherLoaded(bool loadcomplete) { m_WeatherLoaded = loadcomplete; }
	void SetLatitude(double lat) { m_Latitude = lat; }
	double GetLatitude() { return m_Latitude; }

	// Error/Status list access
	void ClearErrorList() { m_ErrorList.RemoveAll(); }
	void ClearInfoList() { m_InformationList.RemoveAll(); }
	void AddToErrorList(CString ErrStg);
	void AddToInfoList(CString InfoStg);
	CStringList* GetErrorList() { return &m_ErrorList; }
	CStringList* GetInfoList() { return &m_InformationList; }

	// Simulation Operations
	COleDateTime GetSimStart() { return m_SimStartTime; }
	COleDateTime GetSimEnd() { return m_SimEndTime; }
	void SetSimStart(COleDateTime start);
	void SetSimEnd(COleDateTime end);
	int GetSimDays();
	int GetSimDayNumber(COleDateTime theDate);
	COleDateTime GetSimDate(int DayNum);
	//int GetNumSeries();
	bool ReadyToSimulate();
	bool IsSimulationComplete() { return m_SimulationComplete; }
	bool AreResultsReady() { return m_ResultsReady; }
	int GetResultsLength() { return m_ResultsText.GetCount(); }
	void Simulate();
	//void UpdateResults(int Day, CEvent* pEvent = NULL);

	// Immigration Operations
	void SetImmigrationType(CString ImType) { m_ImmigrationType = ImType; }
	CString GetImmigrationType() { return m_ImmigrationType; }
	void SetNumImmigrationMites(int mites)
	{
		m_TotImmigratingMites.SetResistant(int(mites * m_ImmMitePctResistant / 100));
		m_TotImmigratingMites.SetNonResistant(mites - m_TotImmigratingMites.GetResistant());
	}
	int GetNumImmigrationMites() { return m_TotImmigratingMites.GetTotal(); }
	void SetImmigrationStart(COleDateTime start) { m_ImmigrationStartDate = start; }
	COleDateTime GetImmigrationStart() { return m_ImmigrationStartDate; }
	void SetImmigrationEnd(COleDateTime end) { m_ImmigrationEndDate = end; }
	COleDateTime GetImmigrationEnd() { return m_ImmigrationEndDate; }
	void SetImmigrationEnabled(bool enabled) { m_ImmigrationEnabled = enabled; }
	bool IsImmigrationEnabled() { return m_ImmigrationEnabled; }
	bool IsImmigrationWindow(CEvent* pEvent);
	double GetImmPctResistant() { return m_ImmMitePctResistant; }
	void SetImmPctResistant(double pctres) { m_ImmMitePctResistant = pctres; }
	//	CMite GetImmigrationMites(COleDateTime theDate);
	CMite GetImmigrationMites(CEvent* pEvent);


	// Implementation
public:
	bool UpdateColonyParameters(CString ParamName, CString ParamValue);
	void InitializeSimulation();

};

/////////////////////////////////////////////////////////////////////////////
