#include "Session.h"

CVarroaPopSession::CVarroaPopSession()
{
	m_DispWeeklyData = false;  // Initially Set to False

	// Defaults for results file
	m_ColTitles = true;
	m_Version = true;
	m_InitConds = false;
	m_WeatherColony = false;
	m_FieldDelimiter = 0; // Fixed width fields




	// Add Header Strings
	m_ResultsHeader.AddTail("Date");
	m_ResultsHeader.AddTail("ColSze");
	m_ResultsHeader.AddTail("AdDrns");
	m_ResultsHeader.AddTail("AdWkrs");
	m_ResultsHeader.AddTail("Forgr");
	m_ResultsHeader.AddTail("DrnBrd");
	m_ResultsHeader.AddTail("WkrBrd");
	m_ResultsHeader.AddTail("DrnLrv");
	m_ResultsHeader.AddTail("WkrLrv");
	m_ResultsHeader.AddTail("DrnEggs");
	m_ResultsHeader.AddTail("WkrEggs");
	m_ResultsHeader.AddTail("TotalEggs");
	m_ResultsHeader.AddTail("DD");
	m_ResultsHeader.AddTail("L");
	m_ResultsHeader.AddTail("N");
	m_ResultsHeader.AddTail("P");
	m_ResultsHeader.AddTail("dd");
	m_ResultsHeader.AddTail("l");
	m_ResultsHeader.AddTail("n");
	m_ResultsHeader.AddTail("FreeMts");
	m_ResultsHeader.AddTail("DBrdMts");
	m_ResultsHeader.AddTail("WBrdMts");
	m_ResultsHeader.AddTail("Mts/DBrd");
	m_ResultsHeader.AddTail("Mts/WBrd");
	m_ResultsHeader.AddTail("Mts Dying");
	m_ResultsHeader.AddTail("PropMts Dying");
	m_ResultsHeader.AddTail("ColPollen(g)");
	m_ResultsHeader.AddTail("PPestConc(ug/g)");
	m_ResultsHeader.AddTail("ColNectar(g)");
	m_ResultsHeader.AddTail("NPestConc(ug/g)");
	m_ResultsHeader.AddTail("Dead DLarv");
	m_ResultsHeader.AddTail("Dead WLarv");
	m_ResultsHeader.AddTail("Dead DAdults");
	m_ResultsHeader.AddTail("Dead WAdults");
	m_ResultsHeader.AddTail("Dead Foragers");
	m_ResultsHeader.AddTail("Queen Strength");
	m_ResultsHeader.AddTail("Temp (DegC)");
	m_ResultsHeader.AddTail("Precip");


	m_ImmigrationType = "None";
	m_TotImmigratingMites = 0;
	m_ImmMitePctResistant = 0;
	m_ImmigrationStartDate = COleDateTime(1999, 1, 1, 0, 0, 0);
	m_ImmigrationEndDate = COleDateTime(1999, 1, 1, 0, 0, 0);
	m_ImmigrationEnabled = false;
	m_SimulationComplete = false;
	m_ResultsReady = false;
	m_RQEggLayingDelay = 10;
	m_RQEnableReQueen = false;
	m_RQScheduled = 1;
	m_RQQueenStrength = 5;
	m_RQOnce = 0;
	m_VTTreatmentDuration = 0;
	m_VTEnable = false;
	m_VTMortality = 0;
	m_InitMitePctResistant = 0;

	m_SPEnable = FALSE;
	m_SPInitial = 0;


	m_CombRemoveDate = COleDateTime(1999, 1, 1, 0, 0, 0);
	m_CombRemoveEnable = FALSE;
	m_CombRemovePct = 0;

	m_pWeather = new CWeatherEvents; // Create the WeatherEvents
	m_WeatherLoaded = false;
	SetShowWarnings(true);

	theColony.SetSession(this);
}

CVarroaPopSession::~CVarroaPopSession()
{
	m_MiteTreatments.ClearAll();
	m_pWeather->ClearAllEvents();
	delete m_pWeather;
}


// Error list and information list access

void CVarroaPopSession::AddToErrorList(CString ErrStg)
{
	m_ErrorList.AddTail(ErrStg);
}

void CVarroaPopSession::AddToInfoList(CString InfoStg)
{
	m_InformationList.AddTail(InfoStg);
}


/////////////////////////////////////////////////////////////////////////////
// CVarroaPopDoc Simulate Functions
void CVarroaPopSession::SetSimStart(COleDateTime start)
{
	m_SimStartTime = start;
	theColony.m_InitCond.m_SimStart = m_SimStartTime.Format("%m/%d/%Y");
}

void CVarroaPopSession::SetSimEnd(COleDateTime end)
{
	m_SimEndTime = end;
	theColony.m_InitCond.m_SimEnd = m_SimEndTime.Format("%m/%d/%Y");
}


bool CVarroaPopSession::ReadyToSimulate()
{
	return(theColony.IsInitialized() && m_pWeather->IsInitialized());
}

int CVarroaPopSession::GetSimDays()
{
	COleDateTimeSpan ts = GetSimEnd() - GetSimStart();
	return (int)ts.GetDays() + 1;
}

int CVarroaPopSession::GetSimDayNumber(COleDateTime theDate)
{
	COleDateTime ss = GetSimStart();
	COleDateTimeSpan ts = theDate - GetSimStart();
	int num = (int)ts.GetDays() + 1;
	return num;
}


COleDateTime CVarroaPopSession::GetSimDate(int DayNumber)
{
	COleDateTimeSpan ts((long)DayNumber, 0, 0, 0);
	return (GetSimStart() + ts);
}



bool CVarroaPopSession::IsImmigrationWindow(CEvent* pEvent)
{
	COleDateTime today = (pEvent->GetTime());

	return((today >= m_ImmigrationStartDate) && (today <= m_ImmigrationEndDate));

}


//CMite CVarroaPopSession::GetImmigrationMites(COleDateTime theDate)
CMite CVarroaPopSession::GetImmigrationMites(CEvent* pEvent)
{
	//  This routine calculates the number of mites to immigrate on the 
	//  specified date.  It also keeps track of the cumulative number of 
	//  mites that have migrated so far.  First calculate the total quantity
	//  of immigrating mites then return a CMite based on percent resistance to miticide
	/*
		The equations of immigration were derived by identifying the desired function,
		e.g. f(x) = A*Cos(x), then calculating the constants by setting the integral of
		the function (over the range 0..1) to 1.  This means that the area under the
		curve is = 1.  This ensures that 100% of m_TotImmigratingMites were added to the
		colony.  With the constants were established, a very simple numerical integration
		is performed using sum(f(x)*DeltaX) for each day of immigration.

		The immigration functions are:

			Cosine -> f(x) = 1.188395*cos(x)

			Sine -> f(x) = 1.57078*sin(PI*x)

			Tangent -> f(x) = 2.648784*tan(1.5*x)

			Exponential -> f(x) = (1.0/(e-2))*(exp(1 - (x)) - 1.0)

			Logarithmic -> f(x) = -1.0*log(x)  day #2 and on

			Polynomial -> f(x) = 3.0*(x) - 1.5*(x*x)

		In the case of Logarithmic, since there is an infinity at x=0, the
		actual value of the integral over the range (0..DeltaX) is used on the first
		day.

		Mites only immigrate on foraging days.



	*/

	double answer;
	COleDateTime theDate = pEvent->GetTime();
	if ((theDate >= GetImmigrationStart()) && (theDate <= GetImmigrationEnd() && pEvent->IsForageDay()))
	{
		int SimDaytoday = GetSimDayNumber(theDate);
		int SimDayImStart = GetSimDayNumber(GetImmigrationStart());
		int SimDayImStop = GetSimDayNumber(GetImmigrationEnd());

		// Set cumulative immigration to 0 on first day
		if (SimDaytoday == SimDayImStart) m_CumImmigratingMites = 0;

		// If today is the last immigration day, immigrate all remaining mites
		// NOTE: Changed this logic in version 3.2.8.10 when we decided to not immigrade on non-foraging days.
		// In that case, it doesn't make sense to immigrate all remaining mites on the last day
		if (false) {} //(SimDaytoday == SimDayImStop) answer = m_TotImmigratingMites - m_CumImmigratingMites;
		else
		{

			// Calculate the proportion of days into immigration
			double ImProp = (double)(SimDaytoday - SimDayImStart) /
				(double)(1 + SimDayImStop - SimDayImStart);

			double DeltaX = 1.0 / (SimDayImStop - SimDayImStart + 1);
			double X = ImProp + DeltaX / 2;


			// Return function based on immigration type
			if (m_ImmigrationType.MakeUpper() == "NONE") answer = 0;

			else if (m_ImmigrationType.MakeUpper() == "COSINE")  // f(x) = A*Cos(x)
			{
				answer = GetNumImmigrationMites() * 1.188395 * cos(X) * DeltaX;
			}

			else if (m_ImmigrationType.MakeUpper() == "EXPONENTIAL") // f(x) = A*(exp(1-x) + B) dx
			{
				answer = GetNumImmigrationMites() * (1.0 / (exp(1.0) - 2)) * (exp(1.0 - (X)) - 1.0) * DeltaX;
			}

			else if (m_ImmigrationType.MakeUpper() == "LOGARITHMIC") // f(x) = A*log(x) dx
			{
				if (ImProp == 0) // Deal with discontinuity at 0
				{
					answer = GetNumImmigrationMites() * (-1.0 * DeltaX * log(DeltaX) - DeltaX);
				}
				else answer = GetNumImmigrationMites() * (-1.0 * log(X) * DeltaX);
			}

			else if (m_ImmigrationType.MakeUpper() == "POLYNOMIAL") // f(x) = (A*x + B*x*x) dx
			{
				answer = GetNumImmigrationMites() * (3.0 * (X)-1.5 * (X * X)) * DeltaX;
			}

			else if (m_ImmigrationType.MakeUpper() == "SINE") // f(x) = A*Sin(PiX) dx
			{
				answer = GetNumImmigrationMites() * 1.57078 * sin(3.1416 * X) * DeltaX;
			}
			else if (m_ImmigrationType.MakeUpper() == "TANGENT") // f(x) = A*Tan(B*X) dx
			{
				answer = GetNumImmigrationMites() * 2.648784 * tan(1.5 * X) * DeltaX;
			}

			else answer = 0;  // m_ImmigrationType not valid


			// Now calculate the correction factor based on number of days of immigration.
			// The equations assume 25 days so correction factor is 25/(#days in sim)
			/*
			if (!(m_ImmigrationType == "Cosine"))
			{
			double CorrFact = (double)25/
				(double)(GetSimDayNumber(GetImmigrationEnd()) -
				GetSimDayNumber(GetImmigrationStart()) + 1);
			answer = answer*CorrFact;
			}
			*/

			// Constrain to positive number
			if (answer < 0.0) answer = 0.0;
		}

		// Increment the running total of mites that have immigrated
		int ResistantMites = int((answer * m_ImmMitePctResistant) / 100 + 0.5);
		m_CumImmigratingMites += CMite(ResistantMites, int(answer - ResistantMites + 0.5));
	}
	else answer = 0;
	CMite theImms;
	theImms.SetResistant(int((answer * m_ImmMitePctResistant) / 100 + 0.5));
	theImms.SetNonResistant(int(answer - theImms.GetResistant() + 0.5));
	return theImms;
}

void CVarroaPopSession::InitializeSimulation()
{
	// Initialize Results Matrix
	/*  The columns of this CMatrix are defined as follows:

				Col 0 = Day Number
				Col 1 = Data point for first series to be plotted
				Col 2 = Data point for second series to be plotted
				Col n = Data point for Nth series to be plotted

		The rows correspond to the values for each day.  The dimensions of the
		CMatrix array are set to (Number of series + 1, Number of days being plotted)
	*/

	
	m_ResultsText.RemoveAll();
	m_ResultsHeader.RemoveAll();
	m_ResultsFileHeader.RemoveAll();
	m_IncImmigratingMites = 0;

	theColony.InitializeColony();
	theColony.SetMiticideTreatment(m_MiteTreatments, m_VTEnable);
	theColony.SetMitePctResistance(m_InitMitePctResistant);

	// Initializing the Spore functions
//	theColony.SetSporeTreatment(GetSimDayNumber(m_SPTreatmentStart),m_SPEnable);
//	theColony.m_Spores.SetMortalityFunction(0.10,,0);
	m_CumImmigratingMites = int(0);
	m_FirstResultEntry = true;



}


void CVarroaPopSession::Simulate()
{
	if (ReadyToSimulate())
	{
		if (!CheckDateConsistency(IsShowWarnings())) return;

		InitializeSimulation();

		//  Set results frequency 
		int ResFreq = m_DispWeeklyData ? 7 : 1;

		const char* formatData[] = {
			"%s", // "Initial or Date"
			"%6d", // Colony size
			"%8d", // Adult Drones
			"%8d", // Adult Workers
			"%8d", // Foragers
			"%8d", // Active Foragers
			"%7d", // Drones Brood
			"%6d", // Wkr Brood
			"%6d", // Drone Larv
			"%6d", // Wkr Larv
			"%6d", // Drone Eggs
			"%6d", // Wkr Eggs
			"%6d", // Total Eggs
			"%7.2f", // DD 
			"%6.2f", // L 
			"%6.2f", // N 
			"%8.2f", // P 
			"%7.2f", // dd 
			"%6.2f", // l 
			"%8.2f", // n 
			"%6d", // Free Mites
			"%6d", // DBrood Mites
			"%6d", // WBrood Mites
			"%6.2f", // DMite / Cell
			"%6.2f", // WMite / Cell
			"%6d", // Mites Dying
			"%6.2f", // Prop Mites Dying
			"%8.1f", // Colony Pollen
			"%6.3f", // Conc Pollen Pest
			"%8.1f", // Colony Nectar
			"%6.3f", // Conc Nectar Pest
			"%6d", // Dead DLarv
			"%6d", // Dead WLarv
			"%6d", // Dead DAdlt
			"%6d", // Dead WAdlt
			"%6d", // Dead Foragers
			"%8.3f", // Queen Strength
			"%8.3f", // Ave Temp
			"%6.3f", // Rain
			"%8.3f", // Min Temp
			"%8.3f", // Max Temp
			"%8.2f", // Daylight Hours
			"%8.2f", // Activity Ratio (Forage Inc)
			"%8d", // Forage Day
			NULL
		};

		// Set results data format string
		char delimiter = ' '; // Space delimited
		if (m_FieldDelimiter == 1)		// Comma Delimited
		{
			delimiter = ',';
		}
		else if (m_FieldDelimiter == 2) // Tab Delimited
		{
			delimiter = '\t';
		}

		int formatDataIdx = 0;
		m_ResultsFileFormatStg = formatData[formatDataIdx++];
		while (formatData[formatDataIdx] != NULL)
		{
			m_ResultsFileFormatStg += delimiter;
			m_ResultsFileFormatStg += formatData[formatDataIdx++];
		}

		CEvent* pEvent = m_pWeather->GetDayEvent(GetSimStart());
		int DayCount = 1;
		int TotSimDays = GetSimDays();
		int TotImMites = 0;
		int TotForagingDays = 0;

		CString CurSize;
		CurSize.Format("                                                        Capped  Capped																												              Prop           Conc            Conc                                             ");
		//m_ResultsFileHeader.AddTail(CurSize);
		m_ResultsText.AddTail(CurSize);
		CurSize.Format("            Colony  Adult     Adult           Active    Drone   Wkr     Drone  Wkr   Drone  Wkr   Total                                                         Free   DBrood WBrood DMite  WMite  Mites  Mites  Colony  Pollen  Colony  Nectar   Dead   Dead   Dead   Dead   Dead    Queen      Ave           Min     Max      Daylight  Forage  Forage");
		//m_ResultsFileHeader.AddTail(CurSize);
		m_ResultsText.AddTail(CurSize);

		CurSize.Format("     Date   Size    Drones    Wkr     Forgrs  Forgrs    Brood   Brood   Larv   Larv  Eggs   Eggs  Eggs      DD      L      N      P       dd       l       n    Mites  Mites  Mites  /Cell  /Cell  Dying  Dying  Pollen  Pest    Nectar  Pest     DLarv  WLarv  DAdlt  WAdlt  Forgrs  Strength   Temp  Rain    Temp    Temp     Hours     Inc     Day");
		// Append additional command name if InOut statistics are required
		//
		// NOTE:  Bypassing GlobalOptions for now - suspect desire is to have all variables available - Verify
		//if (GlobalOptions::Get().ShouldOutputInOutCounts())
		if (true)
			{
			CurSize.Format("%s NewWorkerEggs NewDroneEggs WorkerEggsToLarvae DroneEggsToLarvae WorkerLarvaeToBrood DroneLarvaeToBrood WorkerBroodToAdult DroneBroodToAdult DroneAdultsDying ForagersKilledByPesticides WorkerAdultToForagers WinterMortalityForagersLoss ForagersDying", CurSize);
		}
		//m_ResultsFileHeader.AddTail(CurSize);
		m_ResultsText.AddTail(CurSize);

		CurSize.Format(m_ResultsFileFormatStg,
			//pEvent->GetDateStg("%m/%d/%Y"), 
			"Initial   ", // "Initial or Date"
			theColony.GetColonySize(), // Colony size
			theColony.Dadl.GetQuantity(), // Adult Drones
			theColony.Wadl.GetQuantity(), // Adult Workers
			theColony.foragers.GetQuantity(), // Forgers
			theColony.foragers.GetActiveQuantity(), // Active Forgers
			theColony.CapDrn.GetQuantity(), // Drones Brood
			theColony.CapWkr.GetQuantity(), // Wkr Brood
			theColony.Dlarv.GetQuantity(), // Drone Larv
			theColony.Wlarv.GetQuantity(), // Wkr Larv
			theColony.Deggs.GetQuantity(), // Drone Eggs
			theColony.Weggs.GetQuantity(), // Wkr Eggs
			theColony.GetEggsToday(), // Total Eggs
			theColony.GetDDToday(), // DD 
			theColony.GetLToday(), // L 
			theColony.GetNToday(), // N 
			theColony.GetPToday(), // P 
			theColony.GetddToday(), // dd 
			theColony.GetlToday(), // l 
			theColony.GetnToday(), // n 
			theColony.RunMite.GetTotal(), // Free Mites
			theColony.CapDrn.GetMiteCount(), // DBrood Mites
			theColony.CapWkr.GetMiteCount(), // WBrood Mites
			theColony.CapDrn.GetMitesPerCell(), // DMite / Cell
			theColony.CapWkr.GetMitesPerCell(), // WMite / Cell
			0, // Mites Dying
			0.0, // Prop Mites Dying
			0.0, // Colony Pollen
			0.0, // Conc Pollen Pest
			0.0, // Colony Nectar
			0.0, // Conc Nectar Pest
			0, // Dead DLarv
			0, // Dead WLarv
			0, // Dead DAdlt
			0, // Dead WAdlt
			0, // Dead Foragers
			theColony.queen.GetQueenStrength(), // Queen Strength
			0.0, // Ave Temp
			0.0, // Rain
			0.0, // Min Temp
			0.0, // Max Temp
			0.0, // Daylight Hours
			0.0, // Activity Ratio
			0	 // Forage Day
		);
		// Append additional command name if InOut statistics are required
		//if (GlobalOptions::Get().ShouldOutputInOutCounts())
		if (true)
			{
			CurSize.Format("%s %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d"
				, CurSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		}
		m_ResultsText.AddTail(CurSize);


		//m_Bridge->StartSimulation(*this);

		// *************************************************
		// Main Loop for Simulation
		//
		//
		while ((pEvent != NULL) && (DayCount <= TotSimDays))
		{

			theColony.ReQueenIfNeeded(
				DayCount,
				pEvent,
				m_RQEggLayingDelay,
				m_RQWkrDrnRatio,
				m_RQEnableReQueen,
				m_RQScheduled,
				m_RQQueenStrength,
				m_RQOnce,
				m_RQReQueenDate);

			// Determine if there is feed available and call theColony.SetFeedingDay(t/f);
			// Alternate approach is to pass the feed dates and quantities to the colony and
			// let the colony keep track - probably a better idea since the pollen is consumed

			theColony.UpdateBees(pEvent, DayCount);

			if (IsImmigrationEnabled() && IsImmigrationWindow(pEvent))
			{
				m_IncImmigratingMites = (GetImmigrationMites(pEvent));
				m_IncImmigratingMites.SetPctResistant(m_ImmMitePctResistant);
				theColony.AddMites(m_IncImmigratingMites);
			}
			else m_IncImmigratingMites = 0; // Reset to 0 after Immigration Window Closed


			theColony.UpdateMites(pEvent, DayCount);

			if (m_CombRemoveEnable &&
				(pEvent->GetTime().GetYear() == m_CombRemoveDate.GetYear()) &&
				(pEvent->GetTime().GetMonth() == m_CombRemoveDate.GetMonth()) &&
				(pEvent->GetTime().GetDay() == m_CombRemoveDate.GetDay()))
			{
				theColony.RemoveDroneComb(m_CombRemovePct);
				TRACE("Drone Comb Removed on %s\n", pEvent->GetDateStg());
			}

			theColony.DoPendingEvents(pEvent, DayCount); // Sets colony based on discrete events


			if ((DayCount % ResFreq) == 0) // Print once every ResFreq times thru the loop
			{
				double PropMiteDeath =
					theColony.GetMitesDyingThisPeriod() + theColony.GetTotalMiteCount() > 0 ?
					theColony.GetMitesDyingThisPeriod() /
					double(theColony.GetMitesDyingThisPeriod() + theColony.GetTotalMiteCount()) : 0;

				double ColPollen = theColony.m_Resources.GetPollenQuantity(); // In Grams
				double ColNectar = theColony.m_Resources.GetNectarQuantity();
				double NectarPesticideConc = theColony.m_Resources.GetNectarPesticideConcentration() * 1000000;
				double PollenPesticideConc = theColony.m_Resources.GetPollenPesticideConcentration() * 1000000;  // convert from g/g to ug/g

				CurSize.Format(m_ResultsFileFormatStg,
					pEvent->GetDateStg("%m/%d/%Y"),
					theColony.GetColonySize(),
					theColony.Dadl.GetQuantity(),
					theColony.Wadl.GetQuantity(),
					theColony.foragers.GetQuantity(),
					theColony.foragers.GetActiveQuantity(),
					theColony.CapDrn.GetQuantity(),
					theColony.CapWkr.GetQuantity(),
					theColony.Dlarv.GetQuantity(),
					theColony.Wlarv.GetQuantity(),
					theColony.Deggs.GetQuantity(),
					theColony.Weggs.GetQuantity(),
					theColony.GetEggsToday(),
					theColony.GetDDToday(),
					theColony.GetLToday(),
					theColony.GetNToday(),
					theColony.GetPToday(),
					theColony.GetddToday(),
					theColony.GetlToday(),
					theColony.GetnToday(),
					theColony.RunMite.GetTotal(),
					theColony.CapDrn.GetMiteCount(),
					theColony.CapWkr.GetMiteCount(),
					theColony.CapDrn.GetMitesPerCell(),
					theColony.CapWkr.GetMitesPerCell(),
					theColony.GetMitesDyingThisPeriod(),
					PropMiteDeath,
					ColPollen,
					PollenPesticideConc,
					ColNectar,
					NectarPesticideConc,
					theColony.m_DeadDroneLarvaePesticide,
					theColony.m_DeadWorkerLarvaePesticide,
					theColony.m_DeadDroneAdultsPesticide,
					theColony.m_DeadWorkerAdultsPesticide,
					theColony.m_DeadForagersPesticide,
					theColony.queen.GetQueenStrength(),
					pEvent->GetTemp(),
					pEvent->GetRainfall(),
					pEvent->GetMinTemp(),
					pEvent->GetMaxTemp(),
					pEvent->GetDaylightHours(),
					pEvent->GetForageInc(),
					pEvent->IsForageDay()
				);
				// Append additional command name if InOut statistics are required
//				if (GlobalOptions::Get().ShouldOutputInOutCounts())
				if (true)
					{
					CurSize.Format("%s %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d"
						, CurSize
						, theColony.m_InOutEvent.m_NewWEggs
						, theColony.m_InOutEvent.m_NewDEggs
						, theColony.m_InOutEvent.m_WEggsToLarv
						, theColony.m_InOutEvent.m_DEggsToLarv
						, theColony.m_InOutEvent.m_WLarvToBrood
						, theColony.m_InOutEvent.m_DLarvToBrood
						, theColony.m_InOutEvent.m_WBroodToAdult
						, theColony.m_InOutEvent.m_DBroodToAdult
						, theColony.m_InOutEvent.m_DeadDAdults
						, theColony.m_InOutEvent.m_ForagersKilledByPesticide
						, theColony.m_InOutEvent.m_WAdultToForagers
						, theColony.m_InOutEvent.m_WinterMortalityForagersLoss
						, theColony.m_InOutEvent.m_DeadForagers);
				}
				m_ResultsText.AddTail(CurSize);
			}


			//UpdateResults(DayCount, pEvent);

			if ((DayCount % ResFreq) == 0)
				theColony.SetStartSamplePeriod(); // Get ready for new accumulation period

			DayCount++;

			pEvent->IsForageDay() ? TotForagingDays++ : TotForagingDays;

			pEvent = m_pWeather->GetNextEvent();
		}
		//delete pEvent;
		m_ResultsReady = true;
		m_SimulationComplete = true;


		//m_Bridge->EndSimulation(*this);

		theColony.Clear();
		m_SimulationComplete = false;
	}
}



 
//void CVarroaPopSession::ProcessInputFile(CString FileName)
bool CVarroaPopSession::UpdateColonyParameters(CString theName, CString theVal)
{
	theColony.m_RQQueenStrengthArray.RemoveAll();
	CString Line;
	CString Name = theName.MakeLower();
	Name.Trim();
	CString Value = theVal.MakeLower();
	Value.Trim();
	if (Name == "simstart")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
		m_SimStartTime = tempDate;
		SetSimStart(m_SimStartTime);
		//theColony.m_InitCond.m_SimStart = m_SimStartTime.Format("%m/%d/%Y");  // Have sim start in two places - refactor to just VPDoc

		//m_Bridge->SimulationStartUpdated();
		return true;
	}
	if (Name == "simend")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
		m_SimEndTime = tempDate;
		SetSimEnd(m_SimEndTime);
		//theColony.m_InitCond.m_SimEnd = m_SimEndTime.Format("%m/%d/%Y");

		//m_Bridge->SimulationEndUpdated();
		return true;
	}
	if (Name == "icdroneadults")
	{
		theColony.m_InitCond.m_droneAdultsField = atoi(Value);
		return true;
	}
	if (Name == "icworkeradults")
	{
		theColony.m_InitCond.m_workerAdultsField = atoi(Value);
		return true;
	}
	if (Name == "icdronebrood")
	{
		theColony.m_InitCond.m_droneBroodField = atoi(Value);
		return true;
	}
	if (Name == "icworkerbrood")
	{
		theColony.m_InitCond.m_workerBroodField = atoi(Value);
		return true;
	}
	if (Name == "icdronelarvae")
	{
		theColony.m_InitCond.m_droneLarvaeField = atoi(Value);
		return true;
	}
	if (Name == "icworkerlarvae")
	{
		theColony.m_InitCond.m_workerLarvaeField = atoi(Value);
		return true;
	}
	if (Name == "icdroneeggs")
	{
		theColony.m_InitCond.m_droneEggsField = atoi(Value);
		return true;
	}
	if (Name == "icworkereggs")
	{
		theColony.m_InitCond.m_workerEggsField = atoi(Value);
		return true;
	}
	if (Name == "icqueenstrength")
	{
		theColony.m_InitCond.m_QueenStrength = atof(Value);
		return true;
	}
	if (Name == "icforagerlifespan")
	{
		theColony.m_InitCond.m_ForagerLifespan = atoi(Value);
		return true;
	}
	if (Name == "icdroneadultinfest")
	{
		theColony.m_InitCond.m_droneAdultInfestField = float(atof(Value));
		return true;
	}
	if (Name == "icdronebroodinfest")
	{
		theColony.m_InitCond.m_droneBroodInfestField = float(atof(Value));
		return true;
	}
	if (Name == "icdronemiteoffspring")
	{
		theColony.m_InitCond.m_droneMiteOffspringField = float(atof(Value));
		return true;
	}
	if (Name == "icdronemitesurvivorship")
	{
		theColony.m_InitCond.m_droneMiteSurvivorshipField = float(atof(Value));
		return true;
	}
	if (Name == "icworkeradultinfest")
	{
		theColony.m_InitCond.m_workerAdultInfestField = float(atof(Value));
		return true;
	}
	if (Name == "icworkerbroodinfest")
	{
		theColony.m_InitCond.m_workerBroodInfestField = float(atof(Value));
		return true;
	}
	if (Name == "icworkermiteoffspring")
	{
		theColony.m_InitCond.m_workerMiteOffspring = float(atof(Value));
		return true;
	}
	if (Name == "icworkermitesurvivorship")
	{
		theColony.m_InitCond.m_workerMiteSurvivorship = float(atof(Value));
		return true;
	}

	if (Name == "immtype")
	{
		m_ImmigrationType = Value;
		return true;
	}
	if (Name == "totalimmmites")
	{
		m_TotImmigratingMites = atoi(Value);
		return true;
	}
	if (Name == "pctimmmitesresistant")
	{
		m_ImmMitePctResistant = atof(Value);
		m_TotImmigratingMites.SetPctResistant(m_ImmMitePctResistant);
		return true;
	}
	if (Name == "immstart")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		m_ImmigrationStartDate = theDate;
		return true;
	}
	if (Name == "immend")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		m_ImmigrationEndDate = theDate;
		return true;
	}
	if (Name == "immenabled")
	{
		m_ImmigrationEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "rqegglaydelay")
	{
		m_RQEggLayingDelay = UINT(atoi(Value));
		return true;
	}
	if (Name == "rqwkrdrnratio")
	{
		m_RQWkrDrnRatio = atof(Value);
		return true;
	}
	if (Name == "rqrequeendate")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
		m_RQReQueenDate = tempDate;
		return true;
	}
	if (Name == "rqenablerequeen")
	{
		m_RQEnableReQueen = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "rqscheduled")
	{
		m_RQScheduled = (Value == "true") ? 0 : 1;
		return true;
	}
	if (Name == "rqqueenstrength")
		// This value is placed into an array which is used in Colony::RequeenIfNeeded.  Each occurance of this parameter name in the 
		// input file will result in another requeen strength value being added to the end of the array.
		// Thus rqqueenstrength doesn't overwrite the previous value if supplied more than once
	{
		m_RQQueenStrength = atof(Value);
		theColony.m_RQQueenStrengthArray.Add(m_RQQueenStrength);
		return true;
	}
	if (Name == "rqonce")
	{
		m_RQOnce = (Value == "true") ? 0 : 1;
		return true;
	}
	if (Name == "vttreatmentduration")
	{
		m_VTTreatmentDuration = UINT(atoi(Value));
		return true;
	}
	if (Name == "vtmortality")
	{
		m_VTMortality = UINT(atoi(Value));
		return true;
	}
	if (Name == "vtenable")
	{
		m_VTEnable = (Value == "true") ? TRUE : FALSE;
		return true;
	}
	if (Name == "vttreatmentstart")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verifies this is a valid date
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			m_VTTreatmentStart = tempDate;
		}
		return true;
	}
	if (Name == "initmitepctresistant")
	{
		m_InitMitePctResistant = atof(Value);
		return true;
	}
	if (Name == "ainame")
	{
		// TEST TEST TEST  Be sure pointers all work and no issue with name not found
		//AIItem pItem;
		//AIItem* pEPAItem;
		//pEPAItem = theColony.m_EPAData.GetAIItemPtr(Value);
		//theColony.m_EPAData.SetCurrentAIItem(pEPAItem);
		return true;
	}
	if (Name == "aiadultslope")
	{
		theColony.m_EPAData.m_AI_AdultSlope = atof(Value);
		return true;
	}
	if (Name == "aiadultld50")
	{
		theColony.m_EPAData.m_AI_AdultLD50 = atof(Value);
		return true;
	}
	if (Name == "aiadultslopecontact")
	{
		theColony.m_EPAData.m_AI_AdultSlope_Contact = atof(Value);
		return true;
	}
	if (Name == "aiadultld50contact")
	{
		theColony.m_EPAData.m_AI_AdultLD50_Contact = atof(Value);
		return true;
	}
	if (Name == "ailarvaslope") // note: this was incorrectly "ailarvatslope" in 3.2.5.7
	{
		theColony.m_EPAData.m_AI_LarvaSlope = atof(Value);
		return true;
	}
	if (Name == "ailarvald50")
	{
		theColony.m_EPAData.m_AI_LarvaLD50 = atof(Value);
		return true;
	}
	if (Name == "aikow")
	{
		theColony.m_EPAData.m_AI_KOW = atof(Value);
		return true;
	}
	if (Name == "aikoc")
	{
		theColony.m_EPAData.m_AI_KOC = atof(Value);
		return true;
	}
	if (Name == "aihalflife")
	{
		theColony.m_EPAData.m_AI_HalfLife = atof(Value);
		return true;
	}
	if (Name == "aicontactfactor")
	{
		theColony.m_EPAData.m_AI_ContactFactor = atof(Value);
		return true;
	}
	if (Name == "cl4pollen")
	{
		theColony.m_EPAData.m_C_L4_Pollen = atof(Value);
		return true;
	}
	if (Name == "cl4nectar")
	{
		theColony.m_EPAData.m_C_L4_Nectar = atof(Value);
		return true;
	}
	if (Name == "cl5pollen")
	{
		theColony.m_EPAData.m_C_L5_Pollen = atof(Value);
		return true;
	}
	if (Name == "cl5nectar")
	{
		theColony.m_EPAData.m_C_L5_Nectar = atof(Value);
		return true;
	}
	if (Name == "cldpollen")
	{
		theColony.m_EPAData.m_C_LD_Pollen = atof(Value);
		return true;
	}
	if (Name == "cldnectar")
	{
		theColony.m_EPAData.m_C_LD_Nectar = atof(Value);
		return true;
	}
	if (Name == "ca13pollen")
	{
		theColony.m_EPAData.m_C_A13_Pollen = atof(Value);
		return true;
	}
	if (Name == "ca13nectar")
	{
		theColony.m_EPAData.m_C_A13_Nectar = atof(Value);
		return true;
	}
	if (Name == "ca410pollen")
	{
		theColony.m_EPAData.m_C_A410_Pollen = atof(Value);
		return true;
	}
	if (Name == "ca410nectar")
	{
		theColony.m_EPAData.m_C_A410_Nectar = atof(Value);
		return true;
	}
	if (Name == "ca1120pollen")
	{
		theColony.m_EPAData.m_C_A1120_Pollen = atof(Value);
		return true;
	}
	if (Name == "ca1120nectar")
	{
		theColony.m_EPAData.m_C_A1120_Nectar = atof(Value);
		return true;
	}
	if (Name == "cadpollen")
	{
		theColony.m_EPAData.m_C_AD_Pollen = atof(Value);
		return true;
	}
	if (Name == "cadnectar")
	{
		theColony.m_EPAData.m_C_AD_Nectar = atof(Value);
		return true;
	}
	if (Name == "cforagerpollen")
	{
		theColony.m_EPAData.m_C_Forager_Pollen = atof(Value);
		return true;
	}
	if (Name == "cforagernectar")
	{
		theColony.m_EPAData.m_C_Forager_Nectar = atof(Value);
		return true;
	}
	if (Name == "ipollentrips")
	{
		theColony.m_EPAData.m_I_PollenTrips = (int)atof(Value); // no fail on float input but convert to int
		return true;
	}
	if (Name == "inectartrips")
	{
		theColony.m_EPAData.m_I_NectarTrips = (int)atof(Value);
		return true;
	}
	if (Name == "ipercentnectarforagers")
	{
		theColony.m_EPAData.m_I_PercentNectarForagers = atof(Value);
		return true;
	}
	if (Name == "ipollenload")
	{
		theColony.m_EPAData.m_I_PollenLoad = atof(Value);
		return true;
	}
	if (Name == "inectarload")
	{
		theColony.m_EPAData.m_I_NectarLoad = atof(Value);
		return true;
	}
	if (Name == "foliarenabled")
	{
		theColony.m_EPAData.m_FoliarEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "soilenabled")
	{
		theColony.m_EPAData.m_SoilEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "seedenabled")
	{
		theColony.m_EPAData.m_SeedEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "eapprate")
	{
		theColony.m_EPAData.m_E_AppRate = atof(Value);
		return true;
	}
	if (Name == "esoiltheta")
	{
		theColony.m_EPAData.m_E_SoilTheta = atof(Value);
		return true;
	}
	if (Name == "esoilp")
	{
		theColony.m_EPAData.m_E_SoilP = atof(Value);
		return true;
	}
	if (Name == "esoilfoc")
	{
		theColony.m_EPAData.m_E_SoilFoc = atof(Value);
		return true;
	}
	if (Name == "esoilconcentration")
	{
		theColony.m_EPAData.m_E_SoilConcentration = atof(Value);
		return true;
	}
	if (Name == "eseedconcentration")
	{
		theColony.m_EPAData.m_E_SeedConcentration = atof(Value);
		return true;
	}
	if (Name == "foliarappdate")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_FoliarAppDate = tempDate;
		}
		return true;
	}
	if (Name == "foliarforagebegin")
	{
		COleDateTime theDate;
		theDate.ParseDateTime(Value, VAR_DATEVALUEONLY);
		COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
		theColony.m_EPAData.m_FoliarForageBegin = tempDate;
		return true;
	}
	if (Name == "foliarforageend")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_FoliarForageEnd = tempDate;
		}
		return true;
	}
	if (Name == "soilforagebegin")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_SoilForageBegin = tempDate;
		}
		return true;
	}
	if (Name == "soilforageend")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_SoilForageEnd = tempDate;
		}
		return true;
	}
	if (Name == "seedforagebegin")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_SeedForageBegin = tempDate;
		}
		return true;
	}
	if (Name == "seedforageend")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_EPAData.m_SeedForageEnd = tempDate;
		}
		return true;
	}
	if (Name == "necpolfileenable")
	{
		theColony.m_NutrientCT.m_NutrientContEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "necpolfilename")
	{
		theColony.m_NutrientCT.m_ContaminantFileName = Value;
		return true;
	}
	if (Name == "initcolnectar")
	{
		theColony.m_ColonyNecInitAmount = atof(Value);
		return true;
	}
	if (Name == "initcolpollen")
	{
		theColony.m_ColonyPolInitAmount = atof(Value);
		return true;
	}
	if (Name == "maxcolnectar")
	{
		theColony.m_ColonyNecMaxAmount = atof(Value);
		return true;
	}
	if (Name == "maxcolpollen")
	{
		theColony.m_ColonyPolMaxAmount = atof(Value);
		return true;
	}
	if (Name == "suppollenenable")
	{
		theColony.m_SuppPollenEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "suppollenamount")
	{
		theColony.m_SuppPollen.m_StartingAmount = atof(Value);
		return true;
	}
	if (Name == "suppollenbegin")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_SuppPollen.m_BeginDate = tempDate;
		}
		return true;
	}
	if (Name == "suppollenend")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_SuppPollen.m_EndDate = tempDate;
		}
		return true;
	}
	if (Name == "supnectarenable")
	{
		theColony.m_SuppNectarEnabled = (Value == "true") ? true : false;
		return true;
	}
	if (Name == "supnectaramount")
	{
		theColony.m_SuppNectar.m_StartingAmount = atof(Value);
		return true;
	}
	if (Name == "supnectarbegin")
	{
		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_SuppNectar.m_BeginDate = tempDate;
		}
		return true;
	}
	if (Name == "supnectarend")
	{

		COleDateTime theDate;
		if (theDate.ParseDateTime(Value, VAR_DATEVALUEONLY)) // Verify this Value is a valid date expression
		{
			COleDateTime tempDate(theDate.GetYear(), theDate.GetMonth(), theDate.GetDay(), 0, 0, 0);
			theColony.m_SuppNectar.m_EndDate = tempDate;
		}
		return true;
	}
	if (Name == "foragermaxprop")
	{
		theColony.foragers.SetPropActualForagers(atof(Value));
		return true;
	}
	if (Name == "needresourcestolive")
	{
		theColony.m_NoResourceKillsColony = (Value == "true") ? true : false;
		return true;

	}

	if (Name == "etolxition")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_EggTransitionDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						theColony.m_InitCond.m_EggTransitionDRV.AddItem(StartDateStg, EndDateStg, NumVal);
					}
				}
			}
		}
		return true;
	}
	if (Name == "ltobxition")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_LarvaeTransitionDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						theColony.m_InitCond.m_LarvaeTransitionDRV.AddItem(StartDateStg, EndDateStg, NumVal);
					}
				}
			}
		}
		return true;
	}
	if (Name == "btoaxition")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_BroodTransitionDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						theColony.m_InitCond.m_BroodTransitionDRV.AddItem(StartDateStg, EndDateStg, NumVal);
					}
				}
			}
		}
		return true;
	}
	if (Name == "atofxition")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_AdultTransitionDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						theColony.m_InitCond.m_AdultTransitionDRV.AddItem(StartDateStg, EndDateStg, NumVal);
					}
				}
			}
		}
		return true;
	}
	if (Name == "alifespan")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_AdultLifespanDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						if ((NumVal >= 7) && (NumVal <= 21))  // Adult bee age constraint
						{
							theColony.m_InitCond.m_AdultLifespanDRV.AddItem(StartDateStg, EndDateStg, NumVal);
						}
					}
				}
			}
		}
		return true;
	}
	if (Name == "flifespan")
	{
		if (Value == "clear")
		{
			theColony.m_InitCond.m_ForagerLifespanDRV.ClearAll();
		}
		else
		{
			int curpos = 0;
			double NumVal = 0.0;
			CString StartDateStg = Value.Tokenize(",", curpos);
			if (StartDateStg.GetLength() > 0) // Was Start Date found?
			{
				CString EndDateStg = Value.Tokenize(",", curpos);
				if (EndDateStg.GetLength() > 0) // Was End Date Found?
				{
					CString NumValStg = Value.Tokenize(",", curpos);
					if (NumValStg.GetLength() > 0) // Was the % of survivors found?
					{
						NumVal = atof(NumValStg); //  Note that if NumValStg cannot be converted, NumVal will be set to 0.0
						if ((NumVal >= 0) && (NumVal <= 20))  // Constraint on forager lifespan
						{
							theColony.m_InitCond.m_ForagerLifespanDRV.AddItem(StartDateStg, EndDateStg, NumVal);
						}
					}
				}
			}
		}
		return true;
	}

	if (Name == "etolxitionen")
	{
		theColony.m_InitCond.m_EggTransitionDRV.SetEnabled(Value == "true");
		return true;
	}

	if (Name == "ltobxitionen")
	{
		theColony.m_InitCond.m_LarvaeTransitionDRV.SetEnabled(Value == "true");
		return true;
	}

	if (Name == "btoaxitionen")
	{
		theColony.m_InitCond.m_BroodTransitionDRV.SetEnabled(Value == "true");
		return true;
	}

	if (Name == "atofxitionen")
	{
		theColony.m_InitCond.m_AdultTransitionDRV.SetEnabled(Value == "true");
		return true;
	}

	if (Name == "alifespanen")
	{
		theColony.m_InitCond.m_AdultLifespanDRV.SetEnabled(Value == "true");
		return true;
	}

	if (Name == "flifespanen")
	{
		theColony.m_InitCond.m_ForagerLifespanDRV.SetEnabled(Value == "true");
		return true;
	}

	//Fall through - no match
	{
		//m_Bridge->InputFileUnknownVariable(Name);
		return false;
	}

}


//void CVarroaPopSession::StoreResultsFile(CString PathName)
//{
//	try
//	{
//		CStdioFile theFile(PathName, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
//
//		// Write VarroaPop Version
//		if (m_Version)
//		{
//			CString titlestg;
//			titlestg.Format("Varroa Population Simulation - %s\n", COleDateTime::GetCurrentTime().Format("%b %d,%Y  %I:%M:%S %p"));
//			theFile.WriteString(titlestg);
//			theFile.WriteString(m_Bridge->GetVersion() + "\n\n");
//		}
//		// Write Header Info to file
//		if (m_WeatherColony)
//		{
//			theFile.WriteString("Weather File: " + GetWeatherFileName() + "\n");
//			theFile.WriteString("Colony File:  " + GetColonyFileName() + "\n\n");
//		}
//
//		// Write Initial Conditions to file
//		if (m_InitConds)
//		{
//			CString OutStg;
//			OutStg.Format("Drone Adult Infestation=%4.2f\n", theColony.m_InitCond.m_droneAdultInfestField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Brood Infestation=%4.2f\n", theColony.m_InitCond.m_droneBroodInfestField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Mite Survivorship=%4.2f\n", theColony.m_InitCond.m_droneMiteSurvivorshipField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Mite Offspring=%4.2f\n", theColony.m_InitCond.m_droneMiteOffspringField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Adult Infestation=%4.2f\n", theColony.m_InitCond.m_workerAdultInfestField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Brood Infestation=%4.2f\n", theColony.m_InitCond.m_workerBroodInfestField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Mite Survivorship=%4.2f\n", theColony.m_InitCond.m_workerMiteSurvivorship);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Mite Offspring=%4.2f\n", theColony.m_InitCond.m_workerMiteOffspring);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Adults=%d\n", theColony.m_InitCond.m_droneAdultsField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Brood=%d\n", theColony.m_InitCond.m_droneBroodField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Larvae=%d\n", theColony.m_InitCond.m_droneLarvaeField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Drone Eggs=%d\n", theColony.m_InitCond.m_droneEggsField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Adults=%d\n", theColony.m_InitCond.m_workerAdultsField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Brood=%d\n", theColony.m_InitCond.m_workerBroodField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Larvae=%d\n", theColony.m_InitCond.m_workerLarvaeField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Worker Eggs=%d\n", theColony.m_InitCond.m_workerEggsField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Queen Sperm=%d\n", (int)(theColony.m_InitCond.m_QueenSperm));
//			theFile.WriteString(OutStg);
//			OutStg.Format("Maximum Eggs per Day=%d\n", (int)(theColony.m_InitCond.m_MaxEggs));
//			theFile.WriteString(OutStg);
//			OutStg.Format("Forager Lifespan=%d\n", theColony.m_InitCond.m_ForagerLifespan);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Total Eggs=%d\n", theColony.m_InitCond.m_totalEggsField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily DD=%.2f\n", theColony.m_InitCond.m_DDField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily L=%.2f\n", theColony.m_InitCond.m_LField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily N=%.2f\n", theColony.m_InitCond.m_NField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily P=%.2f\n", theColony.m_InitCond.m_PField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily dd=%.2f\n", theColony.m_InitCond.m_ddField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily l=%.2f\n", theColony.m_InitCond.m_lField);
//			theFile.WriteString(OutStg);
//			OutStg.Format("Daily n=%.2f\n", theColony.m_InitCond.m_nField);
//			theFile.WriteString(OutStg);
//
//			if (m_ImmigrationEnabled) theFile.WriteString("Immigration Enabled=TRUE\n");
//			else  theFile.WriteString("Immigration Enabled=FALSE\n");
//
//			if (m_RQEnableReQueen) theFile.WriteString("Requeening Enabled=TRUE\n");
//			else  theFile.WriteString("Requeening Enabled=FALSE\n");
//
//			if (m_VTEnable) theFile.WriteString("Varroa Treatment Enabled=TRUE\n");
//			else  theFile.WriteString("Varroa Treatment Enabled=FALSE\n");
//
//			if (m_CombRemoveEnable) theFile.WriteString("Comb Removal Enabled=TRUE\n");
//			else  theFile.WriteString("Comb Removal Enabled=FALSE\n");
//
//			theFile.WriteString("\n");
//
//		}
//
//
//
//		// Write Column Headers to file
//		POSITION pos = m_ResultsFileHeader.GetHeadPosition();
//		if (m_ColTitles)
//		{
//			while (pos != NULL)
//			{
//				theFile.WriteString(m_ResultsFileHeader.GetNext(pos) + "\n");
//			}
//		}
//
//		// Now write results to the file
//		pos = m_ResultsText.GetHeadPosition();
//		while (pos != NULL)
//		{
//			theFile.WriteString(m_ResultsText.GetNext(pos) + "\n");
//		}
//		theFile.Close();
//	}
//	catch (CFileException* e)
//	{
//		TCHAR stg[255];
//		e->GetErrorMessage(stg, 255);
//		m_Bridge->OutputFileException(stg);
//	}
//
//}


int CVarroaPopSession::GetDocumentLength()
{
	if (m_ResultsText.IsEmpty()) return 0;
	else return(m_ResultsText.GetCount());
}

bool CVarroaPopSession::CheckDateConsistency(bool ShowWarning)
{
	/*  Checks Dates for Immigration, Varroa Treatment and Re-Queening to verify
		they fall inside the Simulation range.  If not, a warning message is displayed
		and the user is given the opportunity to continue or quit the simulation

		Return value:  True if simulation should continue, False otherwise.  User can
		override consistency check and continue from warning message box.  Otherwise,
		inconsistent times will return false.
	*/

	bool Consistent = true;

	if (ShowWarning)
	{
		CString WarnString = "";
		COleDateTime   ImStart(m_ImmigrationStartDate.GetYear(),
			m_ImmigrationStartDate.GetMonth(),
			m_ImmigrationStartDate.GetDay(), 0, 0, 0);

		COleDateTime     ImEnd(m_ImmigrationEndDate.GetYear(),
			m_ImmigrationEndDate.GetMonth(),
			m_ImmigrationEndDate.GetDay(), 0, 0, 0);

		//  Check all dates of interest.  Flag only if operation enabled

		if (m_RQEnableReQueen && (!DateInRange(m_SimStartTime, m_SimEndTime, m_RQReQueenDate)))
		{
			WarnString += "     ReQueening\n";
			Consistent = false;
		}


		// NOTE:  This block needs to be reworked since we use a list of mite treatment items instead of the single m_VTTreatmentStart.
		//        Fix on c# version.
		/*
		if (m_VTEnable && (!DateInRange(m_SimStartTime, m_SimEndTime, m_VTTreatmentStart)))
		{
			WarnString += "     Varroa Treatment Start\n";
			Consistent = false;
		}

		if (m_VTEnable && (!DateInRange(m_SimStartTime, m_SimEndTime,
			m_VTTreatmentStart + COleDateTimeSpan(m_VTTreatmentDuration*7,0,0,0))))
		{
			WarnString += "     Varroa Treatment End\n";
			Consistent = false;
		}
		*/

		if (m_ImmigrationEnabled && (!DateInRange(m_SimStartTime, m_SimEndTime, ImStart)))
		{
			WarnString += "     Immigration Start\n";
			Consistent = false;
		}

		if (m_ImmigrationEnabled && (!DateInRange(m_SimStartTime, m_SimEndTime, ImEnd)))
		{
			WarnString += "     Immigration End\n";
			Consistent = false;
		}

		// Display warning message if enabled
		if (ShowWarning && !Consistent)
		{
			//Consistent = m_Bridge->CheckDateConsistencyFailed(WarnString);
		}
	}

	return Consistent;
}



bool CVarroaPopSession::DateInRange(COleDateTime StartRange, COleDateTime StopRange, COleDateTime theTime)
{
	return ((theTime >= StartRange) && (theTime <= StopRange));

}

//void CVarroaPopSession::LoadMiteTreatments(CMiteTreatments* theTreatments)
//{
//    if (theTreatments != NULL)
//    {
//        CMiteTreatmentItem theItem;
//        m_MiteTreatments.ClearAll();
//        for (int i = 0; i<theTreatments->GetCount(); i++)
//        {
//            theTreatments->GetItem(i,theItem);
//            m_MiteTreatments.AddItem(theItem);
//        }
//    }
//}
