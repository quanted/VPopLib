#if !defined(AFX_COLONY_H__8C6C41B4_7899_11D2_8D9A_0020AF233A70__INCLUDED_)
#define AFX_COLONY_H__8C6C41B4_7899_11D2_8D9A_0020AF233A70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Colony.h : header file
//

#include "stdafx.h"
#include "bee.h"
#include "egg.h"
#include "queen.h"
#include "larva.h"
#include "brood.h"
#include "adult.h"
#include "mite.h"
#include "weatherevents.h"
#include "spores.h"
#include "mitetreatments.h"
#include "mitetreatmentitem.h"
#include "daterangevalues.h"
#include "colonyresource.h"
#include "epadata.h"
#include "nutrientcontaminationtable.h"
#include "cstring.h"
#include "cuintarray.h"
#include "cmapstringtoob.h"
#include <cstring>



class CColony;  // Forward declaration
class CVarroaPopSession;

/////////////////////////////////////////////////////////////////////////////
// List classes for all bee life stages


/////////////////////////////////////////////////////////////////////////////
//
// CBeelist - this is the base class for all bee list classes.  Contains the functions and attributes common to all
//
class CBeelist : public CObList
{
protected:
	int m_ListLength = 0;
	CColony* m_pColony = NULL;
	static const int m_DefaultPropTransition = 1;
	double m_PropTransition = 0.0;  // The proportion of bees from this list that go to the caboose
							  // A number between 0.0 .. 1.0
	CDateRangeValues* m_pDRVList = NULL;

public:
	CBeelist();
	~CBeelist();
	void SetLength(int len) {m_ListLength = len;}
	int GetLength() {return m_ListLength;}
	int GetQuantity();
	int GetQuantityAt(size_t index);
	int GetQuantityAt(size_t from, size_t to);
	void SetQuantityAt(size_t index, int Quan);
	void SetQuantityAt(size_t from, size_t to, int Quan);
	void SetQuantityAtProportional(size_t from, size_t to, double Proportion);
	void KillAll();
	void RemoveListElements();
	void SetColony(CColony* pCol) {m_pColony = pCol;}
	CColony* GetColony() {return m_pColony;}
	CString Status();
	void FactorQuantity(double factor);
	//void SetQuantityAt(int Quan);
	void SetPropTransition(double Prop) {m_PropTransition = Prop;}
	double GetPropTransition() {return m_PropTransition;}

	static int DroneCount;
	static int ForagerCount;
	static int WorkerCount;


};

/////////////////////////////////////////////////////////////////////////////
//
// CAdultlist - Drones and Workers
//
class CAdultlist : public CBeelist
{
protected:
	CAdult Caboose;
public:

	CAdultlist();
	CAdult& GetCaboose() {return Caboose;}
	void ClearCaboose() {Caboose.Reset();}
	//! Add method simply add theBrood to the Adults without making the adults age
	void Add(CBrood theBrood, CColony* theColony, CEvent* theEvent, bool bWorkder = true);
	void Update(CBrood theBrood, CColony* theColony, CEvent* theEvent, bool bWorkder = true);
	void KillAll();
	void UpdateLength(int len, bool bWorker = true);
	int MoveToEnd(int QuantityToMove, int MinAge);
};



/////////////////////////////////////////////////////////////////////////////
//
// CForagerlistA - An update from CForagerlist (created in version 3.2.8.0) to implement a new data structure logic for foragers.
// The design intent is to maintain the same interface as CForagerlist but implement to the new data structure which is a longer CAdultlist.
//
class CForagerlistA : public CAdultlist
{
private:
	CAdultlist PendingForagers;
	double m_PropActualForagers;
	

public:
	CForagerlistA();
	~CForagerlistA();
	void Update(CAdult theAdult, CColony* theColony, CEvent* theEvent);
	void ClearPendingForagers();
	void KillAll();
	int GetQuantity();  // Total Forarger Quantity including UnemployedForagers
	int GetActiveQuantity(); // Total Forager Quantity minus UnemployedForagers
	int GetUnemployedQuantity();
	void SetLength(int len);
	void SetPropActualForagers(double proportion) { m_PropActualForagers = proportion; }
	double GetPropActualForagers() { return m_PropActualForagers; }
};


/////////////////////////////////////////////////////////////////////////////
//
// CBroodlist - capped brood
//
class CBroodlist : public CBeelist
{
protected:
	CBrood Caboose;
public:
	double GetMitesPerCell();
	void Update(CLarva theLarva);
	double GetMiteCount();
	void KillAll();
	void DistributeMites(CMite theMites);
	float GetPropInfest();
	CBrood GetCaboose() {return Caboose;}
	void ClearCaboose() {Caboose.SetNumber(0);}
};


/////////////////////////////////////////////////////////////////////////////
//
// CLarvalist
//
class CLarvalist : public CBeelist
{
protected:
	CLarva Caboose;
public:
	void AddHead(CLarva* plarv);
	void Update(CEgg theEggs);
	CLarva GetCaboose() {return Caboose;}
	void KillAll();
	void ClearCaboose() {Caboose.SetNumber(0);}
};


/////////////////////////////////////////////////////////////////////////////
//
// CEgglist
//
class CEgglist : public CBeelist
{
protected:
	CEgg Caboose;
public:
	void Update(CEgg theEggs);
	void KillAll();
	CEgg GetCaboose() {return Caboose;}
	void ClearCaboose() {Caboose.SetNumber(0);}
};


/////////////////////////////////////////////////////////////////////////////
// CColony 

struct ColonyInitCond
{
	double	m_droneAdultInfestField = 0.0;
	double	m_droneBroodInfestField = 0.0;
	double	m_droneMiteOffspringField = 0.0;
	double	m_droneMiteSurvivorshipField = 0.0;
	double	m_workerAdultInfestField = 0.0;
	double	m_workerBroodInfestField = 0.0;
	double	m_workerMiteOffspring = 0.0;
	double	m_workerMiteSurvivorship = 0.0;
	int		m_droneAdultsField = 0;
	int		m_droneBroodField = 0;
	int		m_droneEggsField = 0;
	int		m_droneLarvaeField = 0;
	int		m_workerAdultsField = 0;
	int		m_workerBroodField = 0;
	int		m_workerEggsField = 0;
	int		m_workerLarvaeField = 0;
	int		m_totalEggsField = 0;
	double  m_DDField = 0.0;
	double  m_LField = 0.0;
	double  m_NField = 0.0;
	double  m_PField = 0.0;
	double  m_ddField = 0.0;
	double  m_lField = 0.0;
	double  m_nField = 0.0;
	//  From Simulation Initial Conditions
	double	m_QueenStrength = 0.0;
	double	m_QueenSperm = 0.0;
	double	m_MaxEggs = 0.0;
	int		m_ForagerLifespan = 4;
	CString m_SimStart = "";
	CString	m_SimEnd = "";
	CDateRangeValues m_EggTransitionDRV;
	CDateRangeValues m_LarvaeTransitionDRV;
	CDateRangeValues m_BroodTransitionDRV;
	CDateRangeValues m_AdultTransitionDRV;
	CDateRangeValues m_ForagerLifespanDRV;
	CDateRangeValues m_AdultLifespanDRV;
};

struct SupplementalFeedingResource
{
	double m_StartingAmount = 0;  //In Grams
	double m_CurrentAmount = 0;	  //In Grams
	COleDateTime m_BeginDate;
	COleDateTime m_EndDate;
};


//  Durations of each life stage
#define EGGLIFE 3
#define DLARVLIFE 7
#define WLARVLIFE 5
#define DBROODLIFE 14
#define WBROODLIFE 13
#define DADLLIFE 21
#define WADLLIFE 21

// Mite attributes
#define PROPINFSTW 0.08
#define PROPINFSTD 0.92
#define MAXMITES_PER_DRONE_CELL 7  //Per Gloria 3/8/2022 changed from 3
#define MAXMITES_PER_WORKER_CELL 4

//  Discrete Events
#define DE_NONE 1
#define DE_SWARM 2
#define DE_CHALKBROOD 3
#define DE_RESOURCEDEP 4
#define DE_SUPERCEDURE 5
#define DE_PESTICIDE 6

class libvpop_EXPORT CColony //: public CCmdTarget
{
	DECLARE_DYNCREATE(CColony)

	CColony();           // protected constructor used by dynamic creation

// Attributes
protected:
	BOOL GetDiscreteEvents(CString key, CUIntArray*& theArray);
	CString name;
	bool HasBeenInitialized;
	bool m_AdultAgingArmedState;
	int m_VTStart;
	int m_SPStart;
	UINT m_VTDuration;
	UINT m_VTMortality;
	bool m_VTEnable;
	bool m_SPEnable;
	bool m_VTTreatmentActive;
	bool m_SPTreatmentActive;
	double  m_InitMitePctResistant;
	double m_MitesDyingToday;
	int	m_AdultAgeDelayLimit = 24;  // Default is 24
	int m_AdultAgingDelayEggThreshold = 50;  //Default is 50
	int m_DaysSinceEggLayingBegan = m_AdultAgeDelayLimit;  //Reset to 0 when eggs laid == 0 and incremented each day when eggs laid != 0;
	bool m_PollenFeedingDay;  // Specifies this is a day when man-made feed is available.
	bool m_NectarFeedingDay;  // Specifies this is a day when man-made feed is available.
	CVarroaPopSession* m_pSession; //Links back to the session if it is present


	
	// Data structure for discrete events
	CMapStringToOb m_EventMap;


public:
	ColonyInitCond m_InitCond;
	double LongRedux[8]; // Longevity Reduction as a function of mite infestation
	int m_CurrentForagerLifespan;
	CArray<double, double> m_RQQueenStrengthArray;
	CStringList m_ColonyEventList;



	// Bee Attributes
	CQueen queen;
	CForagerlistA foragers;
	CAdultlist Dadl;
	CAdultlist Wadl;
	CBroodlist CapWkr;
	CBroodlist CapDrn;
	CLarvalist Wlarv;
	CLarvalist Dlarv;
	CEgglist Weggs;
	CEgglist Deggs;

	// Free Running Mite state
	CMite RunMite;		// Total # of mites outside of cells
	double PropRMVirgins;// Proportion of current Free Mites that have not yet infested

	//Emerging Mites
	CMite EmergingMitesW;  //Mites which have emerged from worker brood and are ready to be processed
	double PropEmergingVirginsW; //Proportion of emerged mites that have only infested once
	int NumEmergingBroodW; //Number of emerging worker brood on one day
	CMite EmergingMitesD;  //Mites which have emerged from brood and are ready to be processed
	double PropEmergingVirginsD; //Proportion of emerged mites that have only infested once
	int NumEmergingBroodD; //Number of emerging brood on one day


	CSpores m_Spores;	// The spore population for in the colony.

	CMiteTreatments m_MiteTreatmentInfo;  // This is public since CVarroaPopDoc will serialize it based on file version

	CColonyResource m_Resources;				// The stored Pollen and Nectar in the colony
	SupplementalFeedingResource m_SuppPollen;	// Supplemental Feeding.  SAmount in grams
	SupplementalFeedingResource m_SuppNectar;	// Supplemental Feeding.  Amount in grams
	double m_ColonyNecInitAmount;				// Starting amount of Nectar(g) at the beginning of the simulation
	double m_ColonyNecMaxAmount;
	double m_ColonyPolInitAmount;				// Starting amount of Pollen(g) at the beginning of the simulation
	double m_ColonyPolMaxAmount;
	bool m_SuppPollenEnabled;
	bool m_SuppNectarEnabled;
	bool m_SuppPollenAnnual;
	bool m_SuppNectarAnnual;
	bool m_NoResourceKillsColony; // Determines if lack of nectar or pollen will cause colony to die.  If true, colony dies.  User parameter.

	CEPAData m_EPAData;		// EPA-related data structure

	// Additional statistics
	// When the option GlobalOptions::Get().ShouldOutputInOutCounts() is activated 
	// the following count will be appended to the normal output
	struct InOutEvent
	{
		void Reset() {
			memset(this, 0, sizeof(InOutEvent));
		}

		int m_NewWEggs = -1; //!< new worker eggs
		int m_NewDEggs = -1; //!< new drone eggs
		int m_WEggsToLarv = -1; //!< worker eggs moving to larvae
		int m_DEggsToLarv = -1; //!< drone eggs moving to larvae
		int m_WLarvToBrood = -1; //!< worker larvae moving to brood
		int m_DLarvToBrood = -1; //!< drone larvae moving to brood
		int m_WBroodToAdult = -1; //!< worker drone moving to adult
		int m_DBroodToAdult = -1; //!< drone drone moving to adult
		int m_DeadDAdults = -1; //!< drone adult dying
		int m_ForagersKilledByPesticide = -1; //!< forager killed by pesticide
		int m_WAdultToForagers = -1;  //!< worker adult moving to forager
		int m_WinterMortalityForagersLoss = -1; //!< forager dying to due winter mortality
		int m_DeadForagers = -1; //!< forager dying
		double m_PropRedux = -1; // Debug for a double
	};
	InOutEvent m_InOutEvent;


// Operations
protected:
	void SetDefaultInitConditions();

public:
	void Create();
	void SetSession(CVarroaPopSession* pSession) { m_pSession = pSession; }
	double m_MitesDyingThisPeriod;
	double GetMitesDyingThisPeriod();
	void SetStartSamplePeriod();
	double GetTotalMiteCount();
	double GetMitesDyingToday();
	int GetNurseBees();
	int GetAdultAgingDelay() { return m_AdultAgeDelayLimit; }
	void SetAdultAgingDelay(int delay) { m_AdultAgeDelayLimit = delay; }
	int GetAdultAgingDelayEggThreshold() { return m_AdultAgingDelayEggThreshold; }
	void SetAdultAgingDelayEggThreshold(int threshold) { m_AdultAgingDelayEggThreshold = threshold; }
	bool IsAdultAgingDelayArmed() {return m_AdultAgingArmedState;}
	void SetAdultAgingDelayArmed(bool armed_state) { m_AdultAgingArmedState = armed_state; }
	bool IsAdultAgingDelayActive();
	void RemoveDiscreteEvent(CString datestg, UINT EventID);
	void AddDiscreteEvent(CString datestg, UINT EventID);
	void DoPendingEvents(CEvent* pWeatherEvent, int CurSimDay);
	virtual ~CColony();
	void Clear();
	CString GetName() {return name;}
	void SetName(CString stg) {name = stg;}
	void InitializeColony();
	void InitializeBees();
	void InitializeMites();
	void SetInitialized(bool val) {HasBeenInitialized = val;}
	bool IsInitialized() {return HasBeenInitialized;}
	void UpdateBees(CEvent* pEvent, int DayNum);
	void UpdateMites(CEvent* pEvent, int DayNum);
	int GetForagerLifespan() {return m_InitCond.m_ForagerLifespan;}
	void AddMites(CMite NewMites);
	void SetMitePctResistance(double pct) {m_InitMitePctResistant = pct;}
	int GetColonySize();
	void RemoveDroneComb(double pct);
	int GetEggsToday();
	double GetDDToday();
	double GetLToday();
	double GetNToday();
	double GetPToday();
	double GetddToday();
	double GetlToday();
	double GetnToday();
	double GetDaylightHrsToday(CEvent* pEvent);
	void ReQueenIfNeeded(
		int		DayNum,
		CEvent* theEvent,
		UINT	EggLayingDelay,
		double	WkrDrnRatio,
		BOOL	EnableReQueen,
		int		Scheduled,
		double		QueenStrength,
		int		Once,
		COleDateTime	ReQueenDate);
	//void SetMiticideTreatment(
	//	int StartDayNum, 
	//	UINT Duration, 
	//	UINT Mortality, 
	//	BOOL Enable);
	//void SetMiticideTreatment(CMiteTreatments& theTreatments, BOOL Enable);
	void SetVTEnable(bool Value) { m_VTEnable = Value; }
	void SetSporeTreatment(
		int StartDayNum,
		BOOL Enable);
	bool IsPollenFeedingDay(CEvent* pEvent);
	bool IsNectarFeedingDay(CEvent* pEvent);
	void KillColony();
	COleDateTime* GetDayNumDate(int theDayNum);
	void AddEventNotification(CString DateStg, CString Msg);


public:


	//DECLARE_MESSAGE_MAP()

	// Colony Resource Management
	double GetPollenNeeds(CEvent* pEvent);
	double GetNectarNeeds(CEvent* pEvent);
	void InitializeColonyResources(void);
	double GetIncomingNectarQuant(void);
	double GetIncomingNectarPesticideConcentration(int DayNum);
	double GetIncomingPollenQuant(void);
	double GetIncomingPollenPesticideConcentration(int DayNum);

	//EPA Pesticide Dose and Mortality
	void ConsumeFood(CEvent* pEvent, int DayNum);
	void DetermineFoliarDose(int DayNum);
	void ApplyPesticideMortality();
	int ApplyPesticideToBees(CBeelist* pList, int from, int to, double CurrentDose, double MaxDose, double LD50, double Slope);
	int QuantityPesticideToKill(CBeelist* pList, double CurrentDose, double MaxDose, double LD50, double Slope);

public:
	CNutrientContaminationTable m_NutrientCT;
	bool m_NutrientContEnabled;
	int m_DeadWorkerLarvaePesticide;
	int m_DeadDroneLarvaePesticide;
	int m_DeadWorkerAdultsPesticide;
	int m_DeadDroneAdultsPesticide;
	int m_DeadForagersPesticide;
protected:
	void AddPollenToResources(SResourceItem theResource);
public:
	void AddNectarToResources(SResourceItem theResource);
};

/////////////////////////////////////////////////////////////////////////////


#endif // !defined(AFX_COLONY_H__8C6C41B4_7899_11D2_8D9A_0020AF233A70__INCLUDED_)
