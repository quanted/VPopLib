// Colony.cpp : implementation file
//

#include "stdafx.h"
#include "coldstoragesimulator.h"
#include "colony.h"
#include "globaloptions.h"
#include "cstring.h"
#include "cstring.format.h"
#include "coledatetime.h"
#include "session.h"
#include "coblist.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// List Class implementations

/////////////////////////////////////////////////////////////////////////////
//
// CBeelist - this is the base class for all bee list classes.  Contains the functions and attributes common to all
//
int CBeelist::DroneCount = 0;
int CBeelist::ForagerCount = 0;
int CBeelist::WorkerCount = 0;

CBeelist::CBeelist()
{

}

CBeelist::~CBeelist()
{
	while (!IsEmpty()) delete (RemoveHead());
	RemoveAll();
}

int CBeelist::GetQuantity()
{
	int TotalCount=0;

	POSITION pos = GetHeadPosition();

	BOOL mt = IsEmpty();
	//size_t count = GetCount();

	while (pos != NULL) 
	{
		CBee* theBee = (CBee*)GetNext(pos);
		if (theBee->IsAlive()) TotalCount += theBee->number;
	}
	return TotalCount;
}
void CBeelist::KillAll()
{
	POSITION pos = GetHeadPosition();

	while (pos != NULL)
	{
		CBee* theBee = (CBee*)GetNext(pos);
		theBee->Alive=false;
		theBee->SetNumber(0);
	}
}

void CBeelist::RemoveListElements()
{
	while (!IsEmpty()) delete (RemoveHead());
	RemoveAll();
}

// Returns the quantity of bees in this boxcar.  Note this is zero-based so the first boxcar is index = 0
int CBeelist::GetQuantityAt(size_t index)
{
	int Count = 0;
	if ((index < GetCount()) && (index >= 0))
	{
		CBee* theBee = (CBee*)GetAt(FindIndex(static_cast<INT_PTR>(index)));
		Count = theBee->GetNumber();
	}

	return Count;
}

void CBeelist::SetQuantityAt(size_t index, int Quan)
{
	if ((index < GetCount()) && (index >= 0))
	{
		CBee* theBee = (CBee*)GetAt(FindIndex(static_cast<INT_PTR>(index)));
		theBee->SetNumber(Quan);
	}

}


// Returns the sum of quantity of bees in the "from" boxcar thru the "to" boxcar, inclusive.  If "to" is greater than the length
// of the list, the sum stops at the end of the list (last boxcar).  If from == to, the quantity at that point is returned.
int CBeelist::GetQuantityAt(size_t from, size_t to)
{
	int Count = 0;
	size_t ListLength = GetCount();

	if ((from >= 0) && (from < ListLength))
	{
		for (size_t i = from; i <= to; i++)
		{
			if (i >= ListLength) break;  // Exceeded the length of the list
			Count += GetQuantityAt(i);
		}
	}
	return Count;
}

// Evenly divides Quan bees between boxcars from -> to inclusive
void CBeelist::SetQuantityAt(size_t from, size_t to, int Quan)
{
	ASSERT(from <= to);
	size_t ListLength = GetCount();
	if (to > ListLength - 1) to = ListLength - 1;
	int QuanPerBoxcar = static_cast<int>(Quan/(1 + (to-from)));  // divide by number of boxcars

	if ((from >= 0) && (from <= to))
	{
		for (size_t i = from; i <= to; i++) SetQuantityAt(i, QuanPerBoxcar);
	}

}
// Sets the quantity of bees in boxcars between from -> to = Number*Proportion.
void CBeelist::SetQuantityAtProportional(size_t from, size_t to, double Proportion)
{
	ASSERT(from <= to);
	size_t ListLength = GetCount();
	if (to > ListLength - 1) to = ListLength - 1;

	if ((from >= 0) && (from <= to))
	{
		int BoxcarQuant = 0;
		for (size_t i = from; i <= to; i++) 
		{
			BoxcarQuant = GetQuantityAt(i);
			SetQuantityAt(i, (int)(BoxcarQuant * Proportion));
		}
	}

}


// Factor quantity increases or decreases the number of bees 
// in each age group of the list such that New Quantity = Old Quantity * factor
void CBeelist::FactorQuantity(double factor)
{
	POSITION pos = GetHeadPosition();
	CBee* pBee;
	while (pos != NULL)
	{
		pBee = (CBee*)GetNext(pos);
		pBee->number = int(pBee->number*factor);
	}

}


CString CBeelist::Status()
{
	POSITION pos = GetHeadPosition();
	CString StatStg, InterStg;
	int count, boxcar = 1;
	StatStg.Format("Tot BC: %d, ",m_ListLength);

	while (pos!=NULL)
	{
		count = ((CBee*)GetNext(pos))->number;
		InterStg.Format("BC%d: %d, ",boxcar, count);
		StatStg += InterStg;
		boxcar++;
	}
	return StatStg;
}





/////////////////////////////////////////////////////////////////////////////
//
// CForagerlistA - An update from CForagerlist (created in version 3.2.8.0) to implement a new data structure logic for foragers.
// The design intent is to maintain the same interface as CForagerlist but implement to the new data structure which is a longer CAdultlist.
//
CForagerlistA::CForagerlistA() : CAdultlist::CAdultlist()
{
	PendingForagers.RemoveAll();
	m_PropActualForagers = .3;
}

CForagerlistA::~CForagerlistA()
{
	while (!PendingForagers.IsEmpty()) delete(PendingForagers.RemoveHead());
	PendingForagers.RemoveAll();
}


void CForagerlistA::ClearPendingForagers()
{
	while (!PendingForagers.IsEmpty())
	{
		CAdult* temp = (CAdult*)PendingForagers.RemoveHead();
		ASSERT(temp);
		delete(temp);
	}
}

int CForagerlistA::GetQuantity()
{
	int quan = CBeelist::GetQuantity();
	POSITION pos = PendingForagers.GetHeadPosition();
	CAdult* temp;
	while (pos != NULL)
	{
		temp = (CAdult*)PendingForagers.GetNext(pos);
		quan += temp->GetNumber();
	}
	return quan;
}

int CForagerlistA::GetActiveQuantity()
{
	int Quan = GetQuantity();
	if (Quan > 0)
	{
		if (Quan > m_pColony->GetColonySize()*m_PropActualForagers) Quan = (int)(m_pColony->GetColonySize()*m_PropActualForagers);
	}
	return Quan;
}

int CForagerlistA::GetUnemployedQuantity()
{
	return (GetQuantity() - GetActiveQuantity());
}

void CForagerlistA::KillAll()
{
	CBeelist::KillAll();
	//Caboose->SetNumber(0);
}


//void CForagerlistA::Create(const int Length)
//{
//	SetLength(Length);
//}


void CForagerlistA::Update(CAdult theAdult, CColony* theColony, CEvent* theDay)
// The Adult Workers coming in are added to the Forager list.
// If the day is a foraging day, the new adult is pushed onto 
// the Pending Forager list and the list is aged one forage increment(.25, .5, .75, or 1.0)
// Then the Pending Forager list is scanned and any elements that have accumulated
// at least one foraging day are moved to the Forager list.  If the day is not a foraging
// day, the number of number of bees coming in are just added to the number of 
// bees in the first boxcar.  Also, a check is
// done on the foragers to see if they have had their lifetime reduced
// due to varroa infestation.
//
// March 2017:  Update to limit active forager population to a proportion of total colony size.
//
//	Reference Gloria DeGrandi-Hoffman "Establishing the foraging population in VARROAPOP"  3/23/2017
//
// Incoming foragers are all added to the UnemployedForager bucket.  Then a quantity of foragers = PropActiveForagers * ColonySize
// is moved from this bucket and added to the first forager boxcar.  If not enough foragers are available in UnemployedForager then
// move all the foragers in UnemployedForager and be done.
//
// June 2017:  Eliminating the UnemployedForager bucket - reverting back to original data structure.  Will keep track of
// Active forager population as not to exceed the specified percentage of colony population size

{
	// 10% of foragers die over the winter 11/1 - 4/1
#define WINTER_MORTALITY_PER_DAY 0.10/152 // Reduce 10% over the 152 days of winter
	//if (theAdult == NULL)
	//{
	//	// make sure we have an adult box car used to age the foragers list even during winter
	//	theAdult = new CAdult(0);
	//}
	// Change lifespan from that of Worker to that of forager
	WorkerCount--;
	ForagerCount++;
	theAdult.SetLifespan(GetColony()->m_CurrentForagerLifespan);
	theAdult.SetCurrentAge(0.0);

	
	const bool pendingForagersFirst = GlobalOptions::Get().ShouldForagersAlwaysAgeBasedOnForageInc();
	if (pendingForagersFirst)
	{
		// Add the Adult to the Pending Foragers list
		if (PendingForagers.GetCount() == 0)
		{
			CAdult* addAdult = new CAdult();
			addAdult->SetCurrentAge(theAdult.GetCurrentAge()); 
			addAdult->SetForageInc(theAdult.GetForageInc());
			addAdult->SetLifespan(theAdult.GetLifespan());
			addAdult->SetMites(theAdult.GetMites());
			addAdult->SetNumber(theAdult.GetNumber());
			addAdult->SetPropVirgins(theAdult.GetPropVirgins());
			PendingForagers.AddHead(addAdult);
		}
		else
		{
			// check if the latest pending adult is still 0 day old if yes add the numbers
			POSITION pos = PendingForagers.GetHeadPosition();
			CAdult* pendingAdult = (CAdult*)PendingForagers.GetNext(pos);
			if (pendingAdult->GetForageInc() == 0.0)
			{
				pendingAdult->SetNumber(pendingAdult->GetNumber() + theAdult.GetNumber());
				theAdult.Reset();
				ForagerCount--;
			}
			else
			{
				CAdult* addAdult = new CAdult();
				addAdult->SetCurrentAge(theAdult.GetCurrentAge());
				addAdult->SetForageInc(theAdult.GetForageInc());
				addAdult->SetLifespan(theAdult.GetLifespan());
				addAdult->SetMites(theAdult.GetMites());
				addAdult->SetNumber(theAdult.GetNumber());
				addAdult->SetPropVirgins(theAdult.GetPropVirgins());
				PendingForagers.AddHead(addAdult);
			}
		}
	}

	if (theDay->IsForageDay())
	{
		if (!pendingForagersFirst)
		{
			CAdult* addAdult = new CAdult();
			addAdult->SetCurrentAge(theAdult.GetCurrentAge());
			addAdult->SetForageInc(theAdult.GetForageInc());
			addAdult->SetLifespan(theAdult.GetLifespan());
			addAdult->SetMites(theAdult.GetMites());
			addAdult->SetNumber(theAdult.GetNumber());
			addAdult->SetPropVirgins(theAdult.GetPropVirgins());
			PendingForagers.AddHead(addAdult);

		}

		POSITION pos = PendingForagers.GetHeadPosition();
		CAdult* pendingAdult;
		while (pos != NULL) // Increment the forageIncrement for Pending List
		{
			pendingAdult = (CAdult*)PendingForagers.GetNext(pos);
			pendingAdult->SetForageInc(pendingAdult->GetForageInc() + theDay->GetForageInc());
		}
		pos = PendingForagers.GetTailPosition();

		// Here we can have several hives of Foragers that Reach 1 day of age, 
		// we should not make others age more than 2 days

		// Keep information either we should age the foragers by 1 day
		bool addHead = false;
		CAdult* foragersHead = new CAdult();

		POSITION oldpos;
		while (pos != NULL)
		{
			oldpos = pos;  // Save old position for possible deletion
			pendingAdult = (CAdult*)PendingForagers.GetPrev(pos);
			if (pendingAdult->GetForageInc() >= 1.0)
			{
				// At least one hive reached 1 day
				addHead = true;

				// Update the bees count on the foragers head to be added
				foragersHead->SetNumber(foragersHead->GetNumber() + pendingAdult->GetNumber());

				// Remove and delete the current pending adult
				PendingForagers.RemoveAt(oldpos);
				delete pendingAdult;
			}
		}
		// In case we need to age the Foragers, let's add the new head
		if (addHead)
		{
			AddHead(foragersHead);
			delete (RemoveTail());
		}
		else delete foragersHead;
		// If the foragers list is full let's remove the oldest cohort
		if (GetCount() >= m_ListLength + 1)
		{
			Caboose = *((CAdult*)RemoveTail());
			ForagerCount--;
		}
		else Caboose.Reset();
	}
	else if (!pendingForagersFirst)
	{
		if (IsEmpty())
		{
			if (GetLength() > 0)
			{
				CAdult* addAdult = new CAdult();
				addAdult->SetCurrentAge(theAdult.GetCurrentAge());
				addAdult->SetForageInc(theAdult.GetForageInc());
				addAdult->SetLifespan(theAdult.GetLifespan());
				addAdult->SetMites(theAdult.GetMites());
				addAdult->SetNumber(theAdult.GetNumber());
				addAdult->SetPropVirgins(theAdult.GetPropVirgins());
				PendingForagers.AddHead(addAdult);
				AddHead(addAdult); // In special case of 0 lifespan, don't add head
			}
		}
		else
		{
			CAdult* ForagerHead = (CAdult*)GetHead();
			ForagerHead->SetNumber(ForagerHead->GetNumber() + theAdult.GetNumber());
			theAdult.Reset();
			ForagerCount--;
		}
	}

	// Check for lifespan reduction due to mite infestation
	if (true) // Turn on or off
	{
		CAdult* ListAdult;
		double PropRedux;
		POSITION pos = GetHeadPosition();
		int day = WADLLIFE + 1;
		theColony->m_InOutEvent.m_PropRedux = 0;
		
		while (pos != NULL)
		{
			ListAdult = (CAdult*)GetNext(pos);
			if (ListAdult->IsAlive()) // If already killed, don't kill again
			{
				if (ListAdult->GetNumber() <= 0) PropRedux = theColony->LongRedux[0];
				else PropRedux = theColony->LongRedux[int(ListAdult->GetMites().GetTotal() / ListAdult->GetNumber())];
				if (PropRedux > 0)
				{
					theColony->m_InOutEvent.m_PropRedux = ListAdult->GetMites().GetTotal() / ListAdult->GetNumber();  //DEBUG STATEMENT
				}
			if (day>(1 - PropRedux)*(WADLLIFE + theColony->m_CurrentForagerLifespan))
				{
					ListAdult->Kill();
				}
			}
			day++;
		}
	}

	//  Is this a winter mortality day?
	if ((theDay->GetTime().GetMonth() >= 11) || (theDay->GetTime().GetMonth() < 4))
	{
		POSITION pos = GetHeadPosition();
		CAdult* theForager;
		int Number;
		while (pos != NULL)
		{
			theForager = (CAdult*)GetNext(pos);
			Number = theForager->GetNumber();
			int NewNumber = static_cast<int>(Number * (1 - WINTER_MORTALITY_PER_DAY));

			// Update stats for Foragers killed due to winter mortality
			theColony->m_InOutEvent.m_WinterMortalityForagersLoss += Number - NewNumber;

			Number = NewNumber;
			theForager->SetNumber(Number);
		}
	}


}


void CForagerlistA::SetLength(int len)
{
	// Because forager lifespan can change as a function of Date/Time spans, SetLength is different than all the other life stages
	// If the desired length is greater than the current length, new, empty boxcars are added to the list.  If the desired length
	// is shorter than the current length, the excess tail boxcars are removed.
	CAdult* pForager;
	if (len < GetCount())
	{
		while ((GetCount() > len) && (GetCount() > 0)) // Delete all boxcars over new age value
		{
			pForager = (CAdult*)RemoveTail();
			delete pForager;
		}
	}
	else if (len > GetCount())
	{
		while (GetCount() < len)
		{
			pForager = new CAdult(0);
			AddTail(pForager);
		}
	}
	m_ListLength = len;
}


/////////////////////////////////////////////////////////////////////////////
//
// CAdultlist - Drones and Workers
//

CAdultlist::CAdultlist()
{
	Caboose.Reset();
}

void CAdultlist::Add(CBrood theBrood, CColony* theColony, CEvent* theEvent, bool bWorker)
{
	//ASSERT(theBrood);
	{
		CAdult* theAdult = new CAdult(theBrood.GetNumber());
		if (bWorker) WorkerCount++;
		else DroneCount++;
		theAdult->SetMites(theBrood.m_Mites);
		theAdult->SetPropVirgins(theBrood.GetPropVirgins());
		theAdult->SetLifespan(WADLLIFE);
		theBrood.Reset();  // These brood are now  gone

		POSITION pos = GetHeadPosition();
		CAdult* adult = (CAdult*)GetNext(pos);
		if (adult != NULL) // Add the incoming brood to the first boxcar
		{
			adult->SetNumber(adult->GetNumber() + theAdult->GetNumber());
			// Calculate the sum of mites that are virgins
			double v_mites = adult->GetMites().GetTotal() * adult->GetPropVirgins() + theAdult->GetMites().GetTotal() * theAdult->GetPropVirgins();
			adult->SetMites(adult->GetMites() + theAdult->GetMites());
			double propv = (adult->GetMites().GetTotal() > 0) ? adult->GetMites().GetTotal() : 0;
			adult->SetPropVirgins(propv);
			delete theAdult;
		}
		else
		{
			AddHead(theAdult);
		}
	}
}
void CAdultlist::Update(CBrood theBrood, CColony* theColony, CEvent* theEvent, bool bWorker)
// The Capped Brood coming in are converted to Adults and pushed onto the list.
// If the list is now greater than the specified number of days, the
// bottom of the list is removed and assigned to the Caboose for Workers or the 
// bottom of the list is deleted for drones.  We retain the number of mites from
// the Brood so they can be retrieved and released back into the colony.  We also
// establish the maximum lifetime for the new adult worker and see if any other
// workers in the list have extended beyond their lifetime as reduced by varroa infestation
//
// TODO:
//
// When the following are all true (non-foraging day, no brood, no larvae), do not age the adults
{
	{
		CAdult* theAdult = new CAdult(theBrood.GetNumber());
		if (bWorker) WorkerCount++;
		else DroneCount++;
		theAdult->SetMites(theBrood.m_Mites);
		theAdult->SetPropVirgins(theBrood.GetPropVirgins());
		theAdult->SetLifespan(WADLLIFE);

		// Save the emerging mites for use when UpdateMites is called
		if (bWorker)
		{
			theColony->EmergingMitesW = theBrood.m_Mites;
			theColony->PropEmergingVirginsW = theBrood.GetPropVirgins();
			theColony->NumEmergingBroodW = theBrood.GetNumber();
		}
		else
		{
			theColony->EmergingMitesD = theBrood.m_Mites;
			theColony->PropEmergingVirginsD = theBrood.GetPropVirgins();
			theColony->NumEmergingBroodD = theBrood.GetNumber();
		}

		theBrood.Reset();  // These brood are now  gone
		AddHead(theAdult);
		size_t count = GetCount();
		if (GetCount() >= m_ListLength + (int)1) // All Boxcars are full - put workers in caboose, drones die off
		{
			CAdult* pAdult = (CAdult*)RemoveTail();
			Caboose.age = pAdult->age;
			Caboose.Alive = pAdult->Alive;
			Caboose.number = int(pAdult->number * GetPropTransition());
			delete(pAdult);
			if (!bWorker) 
			{
				// Update stats for dead drones
				theColony->m_InOutEvent.m_DeadDAdults = Caboose.GetNumber();

				Caboose.Reset();
				DroneCount--;
			}
		}
		else 
		{
			Caboose.Reset();
		}

		// Check for age beyond reduced lifespan in workers due to mite infestation
		if ((true) && bWorker) // Turn on or off
		{
			double PropRedux;
			POSITION pos = GetHeadPosition();
			int day = 1;
			int index;
			int NumMites;
			while (pos!=NULL)
			{
				theAdult = (CAdult*)GetNext(pos);
				NumMites = theAdult->GetMites();
				if ((theAdult->IsAlive()) && (NumMites > 0) && (theAdult->GetNumber() > 0)) 
				// If already killed, don't kill again, if no mites, ignore
				{
					index = NumMites/theAdult->GetNumber();
					PropRedux = theColony->LongRedux[int(index)];
					if (day>(1-PropRedux)*(theAdult->GetLifespan() + GetColony()->m_CurrentForagerLifespan)) theAdult->Kill();
				}
				day++;
			}
		}
	}
}

void CAdultlist::KillAll()
{
	CBeelist::KillAll();
	//Caboose->SetNumber(0);
}


void CAdultlist::UpdateLength(int len, bool bWorker)
{
	// Because adult house bee lifespan can change as a function of Date/Time spans, SetLength is different than the other life stages
	// If the desired length is greater than the current length, new, empty boxcars are added to the list.  If the desired length
	// is shorter than the current length, the excess bees are added to the caboose so they will become foragers at the next update.
	//
	// SetLength should be called before Update in order to properly move the caboose bees.

	if (bWorker)
	{
		CAdult* pAdult;
		//int count = GetCount();
		if (len < GetCount())
		{
			int AdultsToForagers = 0;
			while((GetCount() > len) && (GetCount() > 0)) // Delete all boxcars over new age value
			{
				pAdult = (CAdult*)RemoveTail();
				AdultsToForagers+= pAdult->GetNumber();
				delete pAdult;      
			}
			Caboose.number += AdultsToForagers;
		}
		else if (len > GetCount()) //Add empty boxcars
		{
			while(GetCount() < len)
			{
				pAdult = new CAdult;            
				AddTail(pAdult);
			}
		}
	}
	m_ListLength = len;
}


// MoveToEnd
// Moves the oldest QuantityToMove bees to the last boxcar.  Does not move any adults younger than MinAge
int CAdultlist::MoveToEnd(int QuantityToMove, int MinAge)
{
	if ((MinAge < 0) || (QuantityToMove <= 0)) return 0;  
	int TotalMoved = 0;
	int CurrentlyMoving = 0;
	int EndIndex = GetLength() - 1;  // The last boxcar in the list
	int index = GetLength() - 2; // Initially points to first boxcar to move - the one just before the last one
	if (QuantityToMove > 0)
	{
		int i = 0;
	}
	while ((TotalMoved < QuantityToMove) && (index >= MinAge))
	{
		CurrentlyMoving = GetQuantityAt(index);
		if (CurrentlyMoving > QuantityToMove - TotalMoved)// In this case, don't move all bees in this boxcar
		{
			CurrentlyMoving = QuantityToMove - TotalMoved;
		}
		SetQuantityAt(EndIndex, CurrentlyMoving + GetQuantityAt(EndIndex));  // Add CurrentlyMoving to the end boxcar
		SetQuantityAt(index, GetQuantityAt(index) -  CurrentlyMoving);  // Remove them from the current boxcar
		// if (CurrentlyMoving > 0) TRACE("Moving Adults to End: %d\n", CurrentlyMoving);
		TotalMoved += CurrentlyMoving;
		index --;
	}
	TRACE("In MoveToEnd %s\n",Status());
	return TotalMoved;
}

/////////////////////////////////////////////////////////////////////////////
//
// CBroodlist - capped brood
//
void CBroodlist::Update(CLarva theLarva)
// The Larva coming in are converted to Capped Brood and pushed onto the list.
// If the list is now greater than the specified number of days, the
// bottom of the list is removed and assigned to the Caboose.
{
	//ASSERT(theLarva);
	CBrood* theBrood = new CBrood((int)theLarva.GetNumber());
	theLarva.Reset();  // These larvae are now  gone
	AddHead(theBrood);
	if (GetCount() >= m_ListLength + (int)1)
	{
		CBrood* tail = (CBrood*)RemoveTail();
		Caboose.age = tail->age;
		Caboose.Alive = tail->Alive;
		Caboose.m_Mites = tail->m_Mites;
		Caboose.SetPropVirgins(tail->GetPropVirgins());
		Caboose.number = int(tail->number * GetPropTransition());
		delete(tail);
	}
	else Caboose.Reset();

}

void CBroodlist::KillAll()
{
	CBeelist::KillAll();
}



double CBroodlist::GetMiteCount()
{
	double TotalCount=0;

	POSITION pos = GetHeadPosition();
	while (pos != NULL) 
	{
		CBrood* theBrood = (CBrood*)GetNext(pos);
		TotalCount += theBrood->m_Mites.GetTotal();
	}
	return TotalCount;
}


float CBroodlist::GetPropInfest()
{
	double TotalUninfested=0, Uninfested = 0;
	int TotalCells = 0;
	float PropInfest;

	POSITION pos = GetHeadPosition();
	while (pos != NULL) 
	{
		CBrood* theBrood = (CBrood*)GetNext(pos);
		Uninfested = theBrood->number - theBrood->m_Mites.GetTotal();
		if (Uninfested < 0) Uninfested = 0;
		TotalCells += theBrood->number;
		TotalUninfested += Uninfested;
	}
	if (TotalCells > 0) PropInfest = 1-(float(TotalUninfested)/float(TotalCells));
	else PropInfest = 0;
	return PropInfest;
}


double CBroodlist::GetMitesPerCell()
{
	return (GetQuantity()>0) ? double(GetMiteCount())/double(GetQuantity()):0.0;

}


void CBroodlist::DistributeMites(CMite theMites)
{
	// Scan thru all the brood boxcars and set mite number
	int Len = GetLength();
	if (Len <= 0) return;
	double MitesPerBoxcar = theMites.GetTotal()/Len;
	double PercentRes = theMites.GetPctResistant();
	POSITION pos;
	CBrood* pBrood;
	pos = GetHeadPosition();
	while (pos != NULL)
	{
		pBrood = (CBrood*)GetNext(pos);
		pBrood->m_Mites = MitesPerBoxcar;
		pBrood->m_Mites.SetPctResistant(PercentRes);
	}
}



/////////////////////////////////////////////////////////////////////////////
//
// CLarvalist
//
void CLarvalist::Update(CEgg theEggs)
// The eggs coming in are converted to Larvae and pushed onto the list.
// If the list is now greater than the specified number of days, the
// bottom of the list is removed and assigned to the Caboose.
{
	CLarva* theLarva = new CLarva((int)theEggs.GetNumber());
	theEggs.Reset();  // These eggs are now  gone
	AddHead(theLarva);
	if (GetCount() >= m_ListLength + (int)1) 
	{
		CLarva* tail = (CLarva*)RemoveTail();
		Caboose.age = tail->age;
		Caboose.Alive = tail->Alive;
		Caboose.number = int(tail->number * GetPropTransition());
		delete(tail);
		
	}
	else Caboose.Reset();

}

void CLarvalist::KillAll()
{
	CBeelist::KillAll();
	//Caboose->SetNumber(0);
}

void CLarvalist::AddHead(CLarva* plarv)
{
	{
		CObList::AddHead(plarv);
		int i = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// CEgglist
//
void CEgglist::Update(CEgg theEggs)
// The eggs laid by the queen on this day are pushed onto the list.
// If the list is now greater than the specified number of days, the
// bottom of the list is removed and assigned to the Caboose.
{
	CEgg* newEggs = new CEgg(theEggs.GetNumber());
	AddHead(newEggs);
	if (GetCount() >= (int)(m_ListLength + 1))  /// NOTE:  Changed from == to >= which shouldn't make a difference but did.  Need to track this down.
	{
		CEgg* tail = (CEgg*)RemoveTail();
		Caboose.age = tail->age;
		Caboose.Alive = tail->Alive;
		Caboose.number = int(tail->number * GetPropTransition());
		delete(tail);
	}
	else Caboose.Reset();

}
void CEgglist::KillAll()
{
	CBeelist::KillAll();
}



/////////////////////////////////////////////////////////////////////////////
// CColony

IMPLEMENT_DYNCREATE(CColony, CCmdTarget)

CColony::CColony()
{
	name = "";
	HasBeenInitialized = false;
	PropRMVirgins = 1.0;

	//  Define table for longevity reduction.  This table is indexed by the
	//  number of mites/cell a given capped brood boxcar.  The reduction is a
	//  proportion based on that infestation rate


/*	LongRedux[0] = 0.0;
	LongRedux[1] = 0.02;
	LongRedux[2] = 0.1;
	LongRedux[3] = 0.2;
	LongRedux[4] = 0.4;
	LongRedux[5] = 0.8;
	LongRedux[6] = 0.9;
	LongRedux[7] = 0.9;*/

	LongRedux[0] = 0.0;
	LongRedux[1] = 0.1;
	LongRedux[2] = 0.2;
	LongRedux[3] = 0.6;
	LongRedux[4] = 0.9;
	LongRedux[5] = 0.9;
	LongRedux[6] = 0.9;
	LongRedux[7] = 0.9;


	m_VTTreatmentActive = false;
	m_VTEnable = false;

	m_SuppPollen.m_CurrentAmount = 0;
	m_SuppPollen.m_StartingAmount = 0;
	m_SuppPollen.m_BeginDate = COleDateTime::GetCurrentTime();
	m_SuppPollen.m_EndDate = COleDateTime::GetCurrentTime();
	m_SuppNectar.m_CurrentAmount = 0;
	m_SuppNectar.m_StartingAmount = 0;
	m_SuppNectar.m_BeginDate = COleDateTime::GetCurrentTime();
	m_SuppNectar.m_EndDate = COleDateTime::GetCurrentTime();

	m_ColonyNecMaxAmount = 0;
	m_ColonyPolMaxAmount = 0;
	m_ColonyNecInitAmount = 0;
	m_ColonyPolInitAmount = 0;

	m_NoResourceKillsColony = false;

}



CColony::~CColony()
{
	Clear();
	if (!m_EventMap.IsEmpty())
	{
		POSITION pos = m_EventMap.GetStartPosition();
		CUIntArray* pArray=NULL;
		CString datestg;
		while (pos != NULL)
		{
			m_EventMap.GetNextAssoc(pos,datestg,(CObject *&)pArray);
			pArray->RemoveAll();
			delete pArray;
			m_EventMap.RemoveKey(datestg);
		}
	}
}




//BEGIN_MESSAGE_MAP(CColony, CCmdTarget)
//	//{{AFX_MSG_MAP(CColony)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColony 


/////////////////////////////////////////////////////////////////////////////
// Initialize Colony
//
// Called at the beginning of each simulation run.  Should reset internal simulation variables and clear lists
// but should not change the initial conditions which are set elsewhere.
//
void CColony::InitializeColony()
{
	InitializeBees();
	InitializeMites();

	// Set pesticide Dose rate to 0
	m_EPAData.m_D_L4 = 0;
	m_EPAData.m_D_L5 = 0;
	m_EPAData.m_D_LD = 0;
	m_EPAData.m_D_A13 = 0;
	m_EPAData.m_D_A410 = 0;
	m_EPAData.m_D_A1120 = 0;
	m_EPAData.m_D_AD = 0;
	m_EPAData.m_D_C_Foragers = 0;
	m_EPAData.m_D_D_Foragers = 0;

	// And set max doses to 0 also
	m_EPAData.m_D_L4_Max = 0;
	m_EPAData.m_D_L5_Max = 0;
	m_EPAData.m_D_LD_Max = 0;
	m_EPAData.m_D_A13_Max = 0;
	m_EPAData.m_D_A410_Max = 0;
	m_EPAData.m_D_A1120_Max = 0;
	m_EPAData.m_D_AD_Max = 0;
	m_EPAData.m_D_C_Foragers_Max = 0;
	m_EPAData.m_D_D_Foragers_Max = 0;

	// Set resources 
	m_Resources.Initialize(m_ColonyPolInitAmount, m_ColonyNecInitAmount);
	m_SuppPollen.m_CurrentAmount = m_SuppPollen.m_StartingAmount;
	m_SuppNectar.m_CurrentAmount = m_SuppNectar.m_StartingAmount;

	// Set pesticide mortality trackers to zero
	m_DeadWorkerLarvaePesticide = 0;
	m_DeadDroneLarvaePesticide = 0;
	m_DeadWorkerAdultsPesticide = 0;
	m_DeadDroneAdultsPesticide = 0;
	m_DeadForagersPesticide = 0;

	m_ColonyEventList.RemoveAll();

	// NOTE: - Need to implement Loading the Contamination Table when we include the pesticide element
	//m_NutrientCT.RemoveAll();
	//if (m_NutrientCT.IsEnabled()) m_NutrientCT.LoadTable(m_NutrientCT.GetFileName());


	//Set the initial state of the AdultAgingDelayArming.  Set armed if the first date is between Jan and Apr inclusive
	auto monthnum = m_pSession->GetSimStart().GetMonth();
	if ((monthnum >= 1) && (monthnum < 3)) SetAdultAgingDelayArmed(true);  // If we start the simulation in Jan or Feb arm Adult Aging Delay
	else SetAdultAgingDelayArmed(false);

	SetInitialized(true);

}

void CColony::AddEventNotification(CString DateStg, CString Msg)
{
	CString EventString = DateStg + ": " + Msg;
	m_pSession->AddToInfoList(EventString);
}



COleDateTime* CColony::GetDayNumDate(int DayNum)
{
	COleDateTime temptime;
	COleDateTime* pReturnDate = NULL;
	COleDateTimeSpan ts((DayNum - 1),0,0,0); // First day is 0 span
	if( temptime.ParseDateTime(m_InitCond.m_SimStart) )
	{
		pReturnDate = new COleDateTime(temptime.GetYear(), temptime.GetMonth(), temptime.GetDay(), 0,0,0);
		*pReturnDate = *pReturnDate + ts;
	}
	
	return pReturnDate;
}

void CColony::KillColony()
{


	// Initialize Queen
	queen.SetStrength(1);

	//  Set lengths of the various lists
	Deggs.KillAll();
	Weggs.KillAll();
	Dlarv.KillAll();
	Wlarv.KillAll();
	CapDrn.KillAll();
	CapWkr.KillAll();
	Dadl.KillAll();
	Wadl.KillAll();
	foragers.KillAll();
	foragers.ClearPendingForagers();
}

void CColony::Create()
{
	Clear();  //Clear all the lists in case they have been build already

	//  Set lengths of the various lists
	Deggs.SetLength(EGGLIFE);
	Deggs.SetPropTransition(1.0);
	Weggs.SetLength(EGGLIFE);
	Weggs.SetPropTransition(1.0);
	Dlarv.SetLength(DLARVLIFE);
	Dlarv.SetPropTransition(1.0);
	Wlarv.SetLength(WLARVLIFE);
	Wlarv.SetPropTransition(1.0);
	CapDrn.SetLength(DBROODLIFE);
	CapDrn.SetPropTransition(1.0);
	CapWkr.SetLength(WBROODLIFE);
	CapWkr.SetPropTransition(1.0);
	Dadl.SetLength(DADLLIFE);
	Dadl.SetPropTransition(1.0);
	Wadl.SetLength(WADLLIFE);
	Wadl.SetPropTransition(1.0);
	Wadl.SetColony(this);
	foragers.SetLength(m_CurrentForagerLifespan);
	foragers.SetColony(this);

	//Remove any current list boxcars in preparation for new initialization
	SetDefaultInitConditions();

}

void CColony::Clear()
{

	//  Empty Wadl list
	while (!Wadl.IsEmpty())
	{
		CAdult* temp = (CAdult*)Wadl.RemoveHead();
		ASSERT(temp);
		delete temp;
		Wadl.ClearCaboose();
	}

	//  Empty Dadl list
	while (!Dadl.IsEmpty())
	{
		CAdult* temp = (CAdult*)Dadl.RemoveHead();
		ASSERT(temp);
		delete temp;
		Dadl.ClearCaboose();
	}

	//  Empty foragers list
	while (!foragers.IsEmpty())
	{
		foragers.ClearPendingForagers();
		CAdult* temp = (CAdult*)foragers.RemoveHead();
		ASSERT(temp);
		delete temp;
		foragers.ClearCaboose();
	}


	//  Empty worker larvae list
	while (!Wlarv.IsEmpty())
	{
		CLarva* temp = (CLarva*)Wlarv.RemoveHead();
		ASSERT(temp);
		delete temp;
		Wlarv.ClearCaboose();
	}

	//  Empty drone larvae list
	while (!Dlarv.IsEmpty())
	{
		CLarva* temp = (CLarva*)Dlarv.RemoveHead();
		ASSERT(temp);
		delete temp;
		Dlarv.ClearCaboose();
	}

	//  Empty drone Brood list
	while (!CapDrn.IsEmpty())
	{
		CBrood* temp = (CBrood*)CapDrn.RemoveHead();
		ASSERT(temp);
		delete temp;
		CapDrn.ClearCaboose();
	}

	//  Empty worker Brood list
	while (!CapWkr.IsEmpty())
	{
		CBrood* temp = (CBrood*)CapWkr.RemoveHead();
		ASSERT(temp);
		delete temp;
		CapWkr.ClearCaboose();
	}


	//  Empty worker egg list
	while (!Weggs.IsEmpty())
	{
		CEgg* temp = (CEgg*)Weggs.RemoveHead();
		ASSERT(temp);
		delete temp;
		Weggs.ClearCaboose();
	}

	//  Empty drone egg list
	while (!Deggs.IsEmpty())
	{
		CEgg* temp = (CEgg*)Deggs.RemoveHead();
		ASSERT(temp);
		delete temp;
		Deggs.ClearCaboose();
	}

	
	// Empty the MiteTreatment list
	m_MiteTreatmentInfo.ClearAll();
	m_RQQueenStrengthArray.RemoveAll();
	queen.SetStrength(m_InitCond.m_QueenStrength);  // Reset this in case requeening happened, reset to original initial condition
}

bool CColony::IsAdultAgingDelayActive()
{
	 //This function updates the Adult Aging parameters related to the part of the Adult aging logic which prevents aging until queen has been laying for a specified number of days.
	 //This function can be called every day but must be called at least once prior to egglaying beginning in the spring and needs to keep being called
	 //daily at least until the delay period has been met.

	// The logic for adult aging delay is as follows:
	// At the end of winter (Jan 1) AdultAgingDelayArmed is set to true meaning we are waiting
	// for the trigger condition:  Total eggs in colony >= 50;
	// Once the trigger condition is set, we set AdultAgingDelayArmed to false indicating we are no longer waiting for the trigger
	// but are in the active delay period during which we don't age adults.
	// Once we have delayed a number of days equal to the limit, this function returns false
	// Indicating we are not in the aging delay state any more.

	//const double EggQuantThreshold = GlobalOptions::Get().AdultAgingDelayEggThreshold();
	const double EggQuantThreshold = GetAdultAgingDelayEggThreshold();

	if (IsAdultAgingDelayArmed())
	{
		if (queen.GetTeggs() > EggQuantThreshold)
		{
			SetAdultAgingDelayArmed(false);
			m_DaysSinceEggLayingBegan = 0;
		}
	}
	auto active = ((m_DaysSinceEggLayingBegan++ < m_AdultAgeDelayLimit) && !IsAdultAgingDelayArmed());

	return active;
}


void CColony::SetDefaultInitConditions()
{
	// This function sets the ICs to values that will allow the model to run without error.

	m_InitCond.m_droneAdultInfestField = 0;
	m_InitCond.m_droneBroodInfestField = 0;
	m_InitCond.m_droneMiteOffspringField = 2.7;
	m_InitCond.m_droneMiteSurvivorshipField = 100;
	m_InitCond.m_workerAdultInfestField = 0;
	m_InitCond.m_workerBroodInfestField = 0;
	m_InitCond.m_workerMiteOffspring = 1.5;
	m_InitCond.m_workerMiteSurvivorship = 100;
	m_InitCond.m_droneAdultsField = 0;
	m_InitCond.m_droneBroodField = 0;
	m_InitCond.m_droneEggsField = 0;
	m_InitCond.m_droneLarvaeField = 0;
	m_InitCond.m_workerAdultsField = 5000;
	m_InitCond.m_workerBroodField = 5000;
	m_InitCond.m_workerEggsField = 5000;
	m_InitCond.m_workerLarvaeField = 5000;
	m_InitCond.m_totalEggsField = 0;

	//  From Simulation Initial Conditions
	m_InitCond.m_QueenStrength = 4;
	m_InitCond.m_ForagerLifespan = 12;
	COleDateTime today = COleDateTime::GetCurrentTime();
	CString m_SimStart = today.Format("%m/%d/%Y");
	COleDateTime later = today += COleDateTimeSpan(30, 0, 0, 0);
	CString	m_SimEnd = later.Format("%m/%d/%Y");

}

void CColony::InitializeBees()
{
	// Set current forager lifespan to the initial condition - could be changed during sim run
	m_CurrentForagerLifespan = m_InitCond.m_ForagerLifespan;

	// Set current Adult aging delay duration when queen restarts egg laying equal to the max duration required so no aging delay happens
	// initially unless egg laying is zero.
	m_DaysSinceEggLayingBegan = m_AdultAgeDelayLimit;

	// Initialize Queen
	queen.SetStrength(m_InitCond.m_QueenStrength);


	foragers.SetLength(m_CurrentForagerLifespan);
	foragers.SetColony(this);

	
	// Distribute bees from initial conditions into age groupings
	// Any rounding error goes into the last boxcar
	int i,e,del;
	
	// Eggs
	e = m_InitCond.m_droneEggsField/Deggs.GetLength();
	del  = m_InitCond.m_droneEggsField - e*Deggs.GetLength();  // Any remainder due to integer truncation
	for (i=0;i<Deggs.GetLength();i++)
	{
		CEgg* theEggs;
		if (i<(Deggs.GetLength()-1)) theEggs = new CEgg(e);
		else  theEggs = new CEgg(e+del); // For the last boxcar, load the average + the trucation number.
		Deggs.AddHead(theEggs);
	}

	e = m_InitCond.m_workerEggsField/Weggs.GetLength();
	del  = m_InitCond.m_workerEggsField - e*Weggs.GetLength();
	for (i=0;i<Weggs.GetLength();i++)
	{
		CEgg* theEggs;
		if (i<(Weggs.GetLength()-1)) theEggs = new CEgg(e);
		else  theEggs = new CEgg(e+del);
		Weggs.AddHead(theEggs);
	}


	// Larvae
	e = m_InitCond.m_droneLarvaeField/Dlarv.GetLength();
	del  = m_InitCond.m_droneLarvaeField - e*Dlarv.GetLength();
	for (i=0;i<Dlarv.GetLength();i++)
	{
		CLarva* theLarva;
		if (i<(Dlarv.GetLength()-1)) theLarva = new CLarva(e);
		else theLarva = new CLarva(e+del);
		Dlarv.AddHead(theLarva);
	}

	e = m_InitCond.m_workerLarvaeField/Wlarv.GetLength();
	del  = m_InitCond.m_workerLarvaeField - e*Wlarv.GetLength();
	for (i=0;i<Wlarv.GetLength();i++)
	{
		CLarva* theLarva;
		if (i<(Wlarv.GetLength()-1)) theLarva = new CLarva(e);
		else theLarva = new CLarva(e+del);
		Wlarv.AddHead(theLarva);
	}

	// Capped Brood
	e = m_InitCond.m_droneBroodField/CapDrn.GetLength();
	del  = m_InitCond.m_droneBroodField - e*CapDrn.GetLength();
	for (i=0;i<CapDrn.GetLength();i++)
	{
		CBrood* theBrood;
		if (i<(CapDrn.GetLength()-1)) theBrood = new CBrood(e);
		else theBrood = new CBrood(e+del);
		CapDrn.AddHead(theBrood);
	}

	e = m_InitCond.m_workerBroodField/CapWkr.GetLength();
	del  = m_InitCond.m_workerBroodField - e*CapWkr.GetLength();
	for (i=0;i<CapWkr.GetLength();i++)
	{
		CBrood* theBrood;
		if (i<(CapWkr.GetLength()-1)) theBrood = new CBrood(e);
		else theBrood = new CBrood(e+del);
		CapWkr.AddHead(theBrood);
	}

	// Drone Adults
	e = m_InitCond.m_droneAdultsField/Dadl.GetLength();
	del  = m_InitCond.m_droneAdultsField - e*Dadl.GetLength();
	for (i=0;i<Dadl.GetLength();i++)
	{
		CAdult* theDrone;
		if (i<(Dadl.GetLength()-1)) theDrone = new CAdult(e);
		else theDrone = new CAdult(e+del);
		theDrone->SetLifespan(DADLLIFE);
		Dadl.AddHead(theDrone);
	}

	// Distribute Worker Adults over Workers and Foragers
	e = m_InitCond.m_workerAdultsField/
	(Wadl.GetLength() + foragers.GetLength());
	del  = m_InitCond.m_workerAdultsField - 
		e*(Wadl.GetLength()+foragers.GetLength());
	for (i=0;i<Wadl.GetLength();i++)  // Distribute into Worker Boxcars
	{
		CAdult* theWorker;
		theWorker = new CAdult(e);
		theWorker->SetLifespan(WADLLIFE);
		Wadl.AddHead(theWorker);
	}
	int ll = foragers.GetLength();
	for (i=0;i<foragers.GetLength();i++) // Distribute remaining into Foragers
	{
		CAdult* theForager;
		if (i<(foragers.GetLength()-1)) theForager = new CAdult(e);
		else theForager = new CAdult(e+del);
		theForager->SetLifespan(foragers.GetLength());
		foragers.AddHead(theForager);
	}



	queen.SetDayOne(1);
	queen.SetEggLayingDelay(0);
}

int CColony::GetColonySize()
{
	return(Dadl.GetQuantity() + Wadl.GetQuantity() + foragers.GetQuantity());
}

void CColony::UpdateBees(CEvent* pEvent, int DayNum)
{

	float LarvPerBee = float(Wlarv.GetQuantity() + Dlarv.GetQuantity()) /
		(Wadl.GetQuantity() + Dadl.GetQuantity() + foragers.GetQuantity());

	if ((pEvent->GetTime().GetMonth() == 1) && pEvent->GetTime().GetDay() == 1) SetAdultAgingDelayArmed(true);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Apply Date Range values
	///////////////////////////////////////////////////////////////////////////////////////////////////
	CString dateStg = pEvent->GetDateStg("%m/%d/%Y");
	COleDateTime theDate;
	if (theDate.ParseDateTime(dateStg)) // The date string was valid
	{
		double PropTransition;

		// Eggs Transition Rate
		if ((m_InitCond.m_EggTransitionDRV.GetActiveValue(theDate, PropTransition)) && (m_InitCond.m_EggTransitionDRV.IsEnabled()))
		{
			Deggs.SetPropTransition(PropTransition / 100);
			Weggs.SetPropTransition(PropTransition / 100);
		}
		else
		{
			Deggs.SetPropTransition(1.0);
			Weggs.SetPropTransition(1.0);
		}
		// Larvae Transition Rate
		if ((m_InitCond.m_LarvaeTransitionDRV.GetActiveValue(theDate, PropTransition)) && (m_InitCond.m_LarvaeTransitionDRV.IsEnabled()))
		{
			Dlarv.SetPropTransition(PropTransition / 100);
			Wlarv.SetPropTransition(PropTransition / 100);
		}
		else
		{
			Dlarv.SetPropTransition(1.0);
			Wlarv.SetPropTransition(1.0);
		}
		// Brood Transition Rate
		if ((m_InitCond.m_BroodTransitionDRV.GetActiveValue(theDate, PropTransition)) && (m_InitCond.m_BroodTransitionDRV.IsEnabled()))
		{
			CapDrn.SetPropTransition(PropTransition / 100);
			CapWkr.SetPropTransition(PropTransition / 100);
		}
		else
		{
			CapDrn.SetPropTransition(1.0);
			CapWkr.SetPropTransition(1.0);
		}
		// Adults Transition Rate
		if ((m_InitCond.m_AdultTransitionDRV.GetActiveValue(theDate, PropTransition)) && (m_InitCond.m_AdultTransitionDRV.IsEnabled()))
		{
			Dadl.SetPropTransition(PropTransition / 100);
			Wadl.SetPropTransition(PropTransition / 100);
		}
		else
		{
			Dadl.SetPropTransition(1.0);
			Wadl.SetPropTransition(1.0);
		}
		// Adults Lifespan Change
		// A reduction moves Adults to caboose in boxcars > new max limit
		// An increase adds empty boxcars up to the new max limit
		double AdultAgeLimit;
		if ((m_InitCond.m_AdultLifespanDRV.GetActiveValue(theDate, AdultAgeLimit)) && (m_InitCond.m_AdultLifespanDRV.IsEnabled()))
		{
			if (Wadl.GetLength() != (int)AdultAgeLimit) // Update if new AdultAgeLimit
			{
				Wadl.UpdateLength((int)AdultAgeLimit);
			}
		}
		else
		{
			if (Wadl.GetLength() != WADLLIFE) // Update if return to default
			{
				Wadl.UpdateLength(WADLLIFE);
			}
		}



		// Foragers Lifespan Change
		// A reduction kills all foragers in boxcars > new max limit
		// An increase adds empty boxcars to the new max limit
		double ForagerLifespan;
		if ((m_InitCond.m_ForagerLifespanDRV.GetActiveValue(theDate, ForagerLifespan)) && (m_InitCond.m_ForagerLifespanDRV.IsEnabled()))
		{
			m_CurrentForagerLifespan = (int)ForagerLifespan;
		}
		else
		{
			m_CurrentForagerLifespan = m_InitCond.m_ForagerLifespan;
		}
		foragers.SetLength(m_CurrentForagerLifespan);


	}
	else   // Invalid date string - reset to default
	{
		Deggs.SetPropTransition(1.0);
		Weggs.SetPropTransition(1.0);
		Dlarv.SetPropTransition(1.0);
		Wlarv.SetPropTransition(1.0);
		CapDrn.SetPropTransition(1.0);
		CapWkr.SetPropTransition(1.0);
		Dadl.SetPropTransition(1.0);
		Wadl.SetPropTransition(1.0);
	}


	// Reset additional output data struct for algorithm intermediate results
	m_InOutEvent.Reset();

	queen.LayEggs(DayNum, pEvent->GetTemp(), pEvent->GetDaylightHours(), foragers.GetQuantity(), LarvPerBee);
	
	// Simulate cold storage
	CColdStorageSimulator& coldStorage = CColdStorageSimulator::Get();
	bool cse = coldStorage.IsEnabled();
	if (cse)
	{
		int i = 1;
	}
	if (coldStorage.IsEnabled())
	{
		if (coldStorage.IsAutomatic())
		{
			if (queen.ComputeL(pEvent->GetDaylightHours()) == 0)
			{
				// In Automatic mode the cold storage is activated if we don't have non adults in the colony anymore ((NOTE:  I don't think this comment is correct
				// cold storage is activated when L = 0 when automatic
				coldStorage.Activate();
			}
			else
			{
				coldStorage.DeActivate();
			}
		}
		coldStorage.Update(*pEvent, *this);
	}


	CEgg l_DEggs(queen.GetDeggs());
	CEgg l_WEggs(queen.GetWeggs());

	// At the beginning of cold storage all eggs are lost
	if (coldStorage.IsStarting())
	{
		l_DEggs.SetNumber(0);
		l_WEggs.SetNumber(0);
	}

	// Update stats for new eggs
	m_InOutEvent.m_NewWEggs = l_WEggs.GetNumber();
	m_InOutEvent.m_NewDEggs = l_DEggs.GetNumber();

	Deggs.Update(l_DEggs);
	Weggs.Update(l_WEggs);

	// At the begining of cold storage no eggs become larvae
	if (coldStorage.IsStarting())
	{
		Weggs.GetCaboose().Reset();
		Deggs.GetCaboose().Reset();
	}

	// Update stats for new larvae
	m_InOutEvent.m_WEggsToLarv = Weggs.GetCaboose().GetNumber();
	m_InOutEvent.m_DEggsToLarv = Deggs.GetCaboose().GetNumber();

	Dlarv.Update(Deggs.GetCaboose());
	Wlarv.Update(Weggs.GetCaboose());

	// At the begining of cold storage no larvae become brood
	if (coldStorage.IsStarting())
	{
		Wlarv.GetCaboose().Reset();
		Dlarv.GetCaboose().Reset();
	}

	// Update stats for new brood
	m_InOutEvent.m_WLarvToBrood = Wlarv.GetCaboose().GetNumber();
	m_InOutEvent.m_DLarvToBrood = Dlarv.GetCaboose().GetNumber();

	CapDrn.Update(Dlarv.GetCaboose());
	CapWkr.Update(Wlarv.GetCaboose());

	// Update stats for new Adults
	m_InOutEvent.m_WBroodToAdult = CapWkr.GetCaboose().GetNumber();
	m_InOutEvent.m_DBroodToAdult = CapDrn.GetCaboose().GetNumber();

	int NumberOfNonAdults = Wlarv.GetQuantity() + Dlarv.GetQuantity() + CapDrn.GetQuantity() + CapWkr.GetQuantity();

	// When the ForageInc is based on temperatures we don't have the aging stopped for Adults as we have during winter 
	// with the default ForageDay election implementation.
	// To correct that, we are saying that a forage day is valid if we have favorable flight hours during that day
	const bool ForageIncIsValid = GlobalOptions::Get().ShouldForageDayElectionBasedOnTemperatures() || pEvent->GetForageInc() > 0.0;

	if ((NumberOfNonAdults > 0) || (pEvent->IsForageDay() && ForageIncIsValid))  
	{
		// Foragers killed due to pesticide.  Recruit precocious Adult Workers to be foragers - Add them to the last Adult Boxcar
		// The last boxcar will be moved to the Caboose when Wadl.Update is called a little later.  The Wadl Caboose will be moved to the 
		// forager list when forager.Update is called.
		//
		int ForagersToBeKilled = QuantityPesticideToKill(&foragers, m_EPAData.m_D_C_Foragers, 0, m_EPAData.m_AI_AdultLD50_Contact, m_EPAData.m_AI_AdultSlope_Contact); //Contact Mortality
		ForagersToBeKilled += QuantityPesticideToKill(&foragers, m_EPAData.m_D_D_Foragers, 0, m_EPAData.m_AI_AdultLD50, m_EPAData.m_AI_AdultSlope); //Diet Mortality
		int MinAgeToForager = 14;
		Wadl.MoveToEnd(ForagersToBeKilled, MinAgeToForager);
		if (ForagersToBeKilled > 0)
		{
			CString notification;
			notification.Format("%d Foragers killed by pesticide - recruiting workers", ForagersToBeKilled);
			AddEventNotification(pEvent->GetDateStg("%m/%d/%Y"), notification);
		}
		// Update stats for foragers killes by pesticides
		m_InOutEvent.m_ForagersKilledByPesticide = ForagersToBeKilled;
		// End Forgers killed due to pesticide

		// Options of aging Adults based on Laid Eggs
		// Aging of adults is actually function of DaylightHours

		bool agingAdults = !coldStorage.IsActive() && (!GlobalOptions::Get().ShouldAdultsAgeBasedLaidEggs() || queen.ComputeL(pEvent->GetDaylightHours()) > 0);

		if (IsAdultAgingDelayActive())
		{
			CString stgDate = pEvent->GetDateStg();
		}
		agingAdults = agingAdults && !IsAdultAgingDelayActive() && !IsAdultAgingDelayArmed();
		if (agingAdults)
		{
			Dadl.Update(CapDrn.GetCaboose(), this, pEvent, false);
			int WkrAdlCabooseNumber = Wadl.GetCaboose().GetNumber();
			Wadl.Update(CapWkr.GetCaboose(), this, pEvent, true);
			int DrnNumberFromCaboose = CapWkr.GetCaboose().GetNumber();
			WkrAdlCabooseNumber = Wadl.GetCaboose().GetNumber();

			// Update stats for adults becoming foragers
			m_InOutEvent.m_WAdultToForagers = Wadl.GetCaboose().GetNumber();

			foragers.Update(Wadl.GetCaboose(), this, pEvent);
		}
		else
		{
			if (NumberOfNonAdults > 0 && GlobalOptions::Get().ShouldAdultsAgeBasedLaidEggs())
			{
				// Let's make sure brood are becoming adults
				Dadl.Add(CapDrn.GetCaboose(), this, pEvent, false);
				Wadl.Add(CapWkr.GetCaboose(), this, pEvent, true);
			}

			// Update stats for new Foragers
			m_InOutEvent.m_WAdultToForagers = 0;

			CAdult resetAdult;
			resetAdult.Reset();
			foragers.Update(resetAdult, this, pEvent);
		}

		// Update stats for dead Foragers
		m_InOutEvent.m_DeadForagers = foragers.GetCaboose().GetNumber() > 0 ? foragers.GetCaboose().GetNumber() : 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Apply the Pesticide Mortality Impacts Here
	//
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	 //TRACE("AIAdultSlope = %f4.2, AIAdultLD50 = %f4.2\n", m_EPAData.m_AI_AdultSlope, m_EPAData.m_AI_AdultLD50);
	ConsumeFood(pEvent, DayNum);
	DetermineFoliarDose(DayNum);
	ApplyPesticideMortality();
	//
	///////////////////////////////////////////////////////////////////////////////////////////////
}


int CColony::GetEggsToday()
{
	return queen.GetTeggs();
}

double CColony::GetDDToday()
{
	return queen.GetDD();
}

double CColony::GetDaylightHrsToday(CEvent* pEvent)
{
	return pEvent->GetDaylightHours();
}

double CColony::GetLToday()
{
	return queen.GetL();
}

double CColony::GetNToday()
{
	return queen.GetN();
}

double CColony::GetPToday()
{
	return queen.GetP();
}

double CColony::GetddToday()
{
	return queen.Getdd();
}

double CColony::GetlToday()
{
	return queen.Getl();
}

double CColony::GetnToday()
{
	return queen.Getn();
}

void CColony::AddMites(CMite NewMites)
{
	//Assume new mites are "virgins"
	CMite virgins = RunMite*PropRMVirgins + NewMites;
	RunMite += NewMites;
	if (RunMite.GetTotal() <=0) PropRMVirgins = 1.0;
	else PropRMVirgins = virgins.GetTotal()/RunMite.GetTotal();
	// Constrain proportion to be [0..1]
	PropRMVirgins = PropRMVirgins > 1 ? 1 : PropRMVirgins;
	PropRMVirgins = PropRMVirgins < 0 ? 0 : PropRMVirgins;

}


void CColony::InitializeMites()
{
	// Initial condition infestation of capped brood
	CMite WMites = CMite(0,int((CapWkr.GetQuantity()*m_InitCond.m_workerBroodInfestField)/float(100)));
	CMite DMites = CMite(0,int((CapDrn.GetQuantity()*m_InitCond.m_droneBroodInfestField)/float(100)));
	CapWkr.DistributeMites(WMites);
	CapDrn.DistributeMites(DMites);

	// Initial condition mites on adult bees i.e. Running Mites
	CMite RunMiteW = CMite(0,int((Wadl.GetQuantity()*m_InitCond.m_workerAdultInfestField+
							foragers.GetQuantity()*m_InitCond.m_workerAdultInfestField)/float(100)));
	CMite RunMiteD = CMite(0,int((Dadl.GetQuantity()*m_InitCond.m_droneAdultInfestField)/float(100)));
	
	RunMite = RunMiteD + RunMiteW;

	//PrevEmergMite = CMite(0,0);
	PropRMVirgins = 1.0;

	m_MitesDyingToday = 0;
	m_MitesDyingThisPeriod = 0;

}


void CColony::UpdateMites(CEvent* pEvent, int DayNum)
{
	/*  Assume UpdateMites is called after UpdateBees.  This means the 
		last Larva boxcar has been moved to the first Brood boxcar and the last
		Brood boxcar has been moved to the first Adult boxcar.  Therefore, we infest
		the first Brood boxcar with mites and we have mites emerging from the
		first boxcar in the appropriate Adult list.
	*/

	/*
	 * The proportion of running mites that have not infested before (PropVirgins) is maintained and updated each day.
	 * The mites with that proportion infest each day and the proportion is updated at the end of this function 
	 */

	/*  New 2/10/22
	 *	Based on recently changed aging logic, the bees in the first adult boxcar may not be new this day.  
	 *	Logic needs to have only new mites emerging and after they are accounted for, set the
	 *	set the mite count in the first adult boxcar to zero.
	 *	i.e. Mites keep emerging every day even if adults don't age.
	 */

	/*  
		Initial step is to infest the larval cells with the free mites just
		before they are capped.  

	*/

	//int i = 1;
	//CString datetoday = pEvent->GetDateStg();
	//if(pEvent->GetDateStg() == "01/01/1964")
	//{
	//	m_MitesDyingToday = 10;
	//}

	m_MitesDyingToday = 0;

	CBrood* WkrBrood = ((CBrood*)CapWkr.GetHead()); // The cells being infested
	CBrood* DrnBrood = ((CBrood*)CapDrn.GetHead()); // in this time cycle

	//  Calculate proportion of RunMites that can invade cells (per Calis)
	double I;
	double B = GetColonySize()*0.125; // Weight in grams of colony
	if (B > 0.0)
	{
		double rD = 6.49*DrnBrood->GetNumber()/B;
		double rW = 0.56*WkrBrood->GetNumber()/B;
		I = (1-exp(-(rD+rW)));
		if (I<0.0)	I = 0.0;
	}
	else 
	{
		I = 0.0;
	}


	CMite WMites = RunMite * (I * PROPINFSTW); // Mite candidates going into worker brood cells

	// Modify potential Drone Mite infestation rate by factoring in likelihood of finding a drone target cell in all the cells
	double Likelihood = 1.0;
	if (WkrBrood->GetNumber() > 0) // Prevent divide by zero
	{
		Likelihood = (double)DrnBrood->GetNumber()/(double)WkrBrood->GetNumber();
		if (Likelihood > 1.0) Likelihood = 1.0;
	}

	CMite OverflowMax;
	CMite OverflowLikelihood;
	CMite DMites = RunMite * (I * PROPINFSTD * Likelihood); // Mites going into drone brood cells
	if (WkrBrood->GetNumber() == 0)
	{
		// Send the WMites to become Drone mite candidates since no worker targets
		DMites += WMites;
		WMites.SetResistant(0);
		WMites.SetNonResistant(0);
	}

	// Capture the number of mites targeted to drone brood but filtered out due to likelihood function
	OverflowLikelihood = RunMite * (I * PROPINFSTD * (1.0 - Likelihood));
	OverflowLikelihood.SetPctResistant(DMites.GetPctResistant());

	//  Determine if too many mites/drone cell.  If so send excess to worker cells
	if (DMites.GetTotal() > MAXMITES_PER_DRONE_CELL*DrnBrood->GetNumber())
	{
		OverflowMax = DMites.GetTotal() - (MAXMITES_PER_DRONE_CELL*DrnBrood->GetNumber());
		OverflowMax.SetPctResistant(DMites.GetPctResistant());
		DMites -= OverflowMax;
	}

	// Add all the overflow mites to those available to infest worker brood
	WMites += OverflowMax + OverflowLikelihood;

	//  Determine if too many mites/worker cell.  If so, limit
	if (WMites.GetTotal() > MAXMITES_PER_WORKER_CELL*WkrBrood->GetNumber()) 
	{
		double pr = WMites.GetPctResistant();
		WMites = MAXMITES_PER_WORKER_CELL*WkrBrood->GetNumber();
		WMites.SetPctResistant(pr);
	}
	RunMite = RunMite - WMites - DMites;
	if (RunMite.GetTotal()<0) RunMite = 0;

	WkrBrood->m_Mites = WMites;
	WkrBrood->SetPropVirgins(PropRMVirgins);
	DrnBrood->m_Mites = DMites;
	DrnBrood->SetPropVirgins(PropRMVirgins);


	/*
		Now we determine the emerging mites from the first boxcar in
		the adult lists

	*/



	// Set the correct values for numbers of cells and numbers of mites
	// emerging this time
	CBrood WkrEmerge;
	CBrood DrnEmerge;
	bool WMitesCounted = ((CAdult*)Wadl.GetHead())->HaveMitesBeenCounted();
	bool DMitesCounted = ((CAdult*)Dadl.GetHead())->HaveMitesBeenCounted();
	WkrEmerge.number = ((CAdult*)Wadl.GetHead())->GetNumber();
	WkrEmerge.m_Mites = ((CAdult*)Wadl.GetHead())->GetMites();
	WkrEmerge.SetPropVirgins(((CAdult*)Wadl.GetHead())->GetPropVirgins());
	DrnEmerge.number = ((CAdult*)Dadl.GetHead())->GetNumber();
	DrnEmerge.m_Mites = ((CAdult*)Dadl.GetHead())->GetMites();
	DrnEmerge.SetPropVirgins(((CAdult*)Dadl.GetHead())->GetPropVirgins()); //Sometimes this is -nan
	if (WMitesCounted)
	{
		// Don't count them again
		WkrEmerge.m_Mites = CMite(0, 0);
	}
	else
	{
		// Count them and mark as counted
		WkrEmerge.m_Mites = ((CAdult*)Wadl.GetHead())->GetMites();
		((CAdult*)Wadl.GetHead())->SetMitesCounted(true);
	}

	if (DMitesCounted)
	{
		// Don't count them again
		DrnEmerge.m_Mites = CMite(0, 0);
	}
	else
	{
		// Count them and mark as counted
		DrnEmerge.m_Mites = ((CAdult*)Dadl.GetHead())->GetMites();
		((CAdult*)Dadl.GetHead())->SetMitesCounted(true);
	}





	double MitesPerCellW;
	double MitesPerCellD;
	if (WkrEmerge.GetNumber() == 0) MitesPerCellW = 0;
	else MitesPerCellW = WkrEmerge.m_Mites.GetTotal()/WkrEmerge.GetNumber();

	if (DrnEmerge.GetNumber() == 0) MitesPerCellD = 0;
	else MitesPerCellD = DrnEmerge.m_Mites.GetTotal()/DrnEmerge.GetNumber();

	// Calculate survivorship
	double PropSurviveMiteW = m_InitCond.m_workerMiteSurvivorship/100;  
	double PropSurviveMiteD = m_InitCond.m_droneMiteSurvivorshipField/100;  
	
	// Calculate reproduction rates per mite per cell
	double ReproMitePerCellD;
	double ReproMitePerCellW;


	if (MitesPerCellW <= 1.0) ReproMitePerCellW = 
		m_InitCond.m_workerMiteOffspring;
	else ReproMitePerCellW = 
			double((1.15*MitesPerCellW) - (0.233*MitesPerCellW*MitesPerCellW));
	if (ReproMitePerCellW < 0) ReproMitePerCellW = 0;


	if (MitesPerCellD <= 2.0) ReproMitePerCellD = // Updated from 1.0 to 2.0 mites per cell use initial conditions - 3/8/2022 per Gloria
		m_InitCond.m_droneMiteOffspringField;
	else ReproMitePerCellD = 
			double(1.734 - (0.0755*MitesPerCellD) - (0.0069*MitesPerCellD*MitesPerCellD));  // Updated equation 3/8/2022 per Gloria
	if (ReproMitePerCellD < 0) ReproMitePerCellD = 0;
 
	// Calculate the number of newly emerging mites consisting of survivors from infestation and new offspring
	#define PROPRUNMITE2 0.6

	CMite SurviveMitesW = WkrEmerge.m_Mites * PropSurviveMiteW;
	CMite SurviveMitesD = DrnEmerge.m_Mites * PropSurviveMiteD;

	double NumEmergingMites = SurviveMitesW.GetTotal() + SurviveMitesD.GetTotal();  
	
	CMite NewMitesW = SurviveMitesW * ReproMitePerCellW;
	CMite NewMitesD = SurviveMitesD * ReproMitePerCellD;

	// Only mites which hadn't previously infested can survive to infest again.
	SurviveMitesW = SurviveMitesW * WkrEmerge.GetPropVirgins();
	SurviveMitesD = SurviveMitesD * DrnEmerge.GetPropVirgins();

	double NumVirgins = SurviveMitesW.GetTotal() + SurviveMitesD.GetTotal();


	CMite RunMiteVirgins = RunMite*PropRMVirgins;
	CMite RunMiteW = NewMitesW + SurviveMitesW * PROPRUNMITE2;
	CMite RunMiteD = NewMitesD + SurviveMitesD * PROPRUNMITE2;

	// Mites dying today are the number which originally emerged from brood minus the ones that eventually became running mites
	m_MitesDyingToday = WkrEmerge.m_Mites.GetTotal() + DrnEmerge.m_Mites.GetTotal();
	m_MitesDyingToday = (m_MitesDyingToday >= 0) ? m_MitesDyingToday : 0; // Constrain positive 

	RunMite = RunMite + RunMiteD + RunMiteW;
	if (RunMite.GetTotal() <= 0)
	{
		PropRMVirgins = 1.0;
	}
	else
	{
		PropRMVirgins = (RunMiteVirgins.GetTotal() + NewMitesW.GetTotal() + NewMitesD.GetTotal()) / RunMite.GetTotal();
		// Constrain proportion to be [0..1]
		PropRMVirgins = PropRMVirgins > 1 ? 1 : PropRMVirgins;
		PropRMVirgins = PropRMVirgins < 0 ? 0 : PropRMVirgins;
	}

	// Kill NonResistant Running Mites if Treatment Enabled

	if (m_VTEnable)
	{
		CMiteTreatmentItem theItem;
		COleDateTime* theDate = GetDayNumDate(DayNum);
		if (m_MiteTreatmentInfo.GetActiveItem(*theDate,theItem))
		{
			double Quan = RunMite.GetTotal();
			RunMite.SetNonResistant(RunMite.GetNonResistant()*
												(100.0 - theItem.PctMortality)/100.0);
			m_MitesDyingToday += (Quan - RunMite.GetTotal());
		}

		delete theDate;
	}
	m_MitesDyingThisPeriod += m_MitesDyingToday;
}
	

void CColony::
ReQueenIfNeeded(
			int		SimDayNum,
			CEvent* theEvent,
			UINT	EggLayingDelay,
			double	WkrDrnRatio,
			BOOL	EnableReQueen,
			int		Scheduled,	// Scheduled == 0,  Automatic != 0
			double		QueenStrength,
			int		RQOnce,		// Once == 0, Annually != 0
			COleDateTime	ReQueenDate)
{
	/* 
	
	There are 2 types of re-queening, 1) Scheduled or 2) Automatic

	For an Automatic re-queening, the re-queening is triggered when the 
	proportion of non-fertilized eggs falls below a certain level but only 
	between April and September of the year, inclusive.

	For Scheduled re-queening, the event is initially triggered on the ReQueenDate.  
	Subsequent re-queenings occur annually if RQOnce is != 0.  Initial re-queening
	occurs when the Year, Month, and Day of the ReQueenDate and the current simulation 
	date are equal.  Subsequent annual re-queening occurs when the Month and Day of 
	the ReQeenDate and the current simulation date are equal.

	Once re-queening happens, the new queen has the specified QueenStrength but egg-
	laying is delayed EggLayingDelay days beyond the date of the re-queening.

	No re-queening occurs if EnableReQueen is false

	*/

	double AppliedStrength = QueenStrength;
	if (!EnableReQueen) return;
	if (Scheduled==0)
	{
		if (  // Initial re-queening
			((ReQueenDate.GetYear()==theEvent->GetTime().GetYear()) &&
			(ReQueenDate.GetMonth()==theEvent->GetTime().GetMonth()) &&
			(ReQueenDate.GetDay()==theEvent->GetTime().GetDay())) ||

			  // Subsequent re-queening
			((ReQueenDate.GetYear()<theEvent->GetTime().GetYear()) &&
			(ReQueenDate.GetMonth()==theEvent->GetTime().GetMonth()) &&
			(ReQueenDate.GetDay()==theEvent->GetTime().GetDay()) &&
			(RQOnce != 0)))
		{
			if (!m_RQQueenStrengthArray.IsEmpty()) // Pop the next strength off the array
			{
				AppliedStrength = m_RQQueenStrengthArray[0];
				m_RQQueenStrengthArray.RemoveAt(0);
			}
			CString notification;
			notification.Format("Scheduled Requeening Occurred, Strength %5.1f",AppliedStrength);
			AddEventNotification(theEvent->GetDateStg("%m/%d/%Y"),notification);
			queen.ReQueen(EggLayingDelay, AppliedStrength, SimDayNum);
		}
	}
	else  // Automatic re-queening
	{
		if ((queen.GetPropDroneEggs() > 0.15 ) &&
			(theEvent->GetTime().GetMonth()>3) && 
			(theEvent->GetTime().GetMonth()<10))
		{
			if (!m_RQQueenStrengthArray.IsEmpty()) // Pop the next strength off the array
			{
				AppliedStrength = m_RQQueenStrengthArray[0];
				m_RQQueenStrengthArray.RemoveAt(0);
			}
			CString notification;
			notification.Format("Automatic Requeening Occurred, Strength %5.1f",AppliedStrength);
			AddEventNotification(theEvent->GetDateStg("%m/%d/%Y"),notification);
			queen.ReQueen(EggLayingDelay, AppliedStrength, SimDayNum);
		}
	}



}

//void CColony::SetMiticideTreatment(int StartDayNum, UINT Duration, UINT Mortality, BOOL Enable)
//{
//	if (Enable)
//	{
//		m_VTStart = StartDayNum; 
//		m_VTDuration = Duration; 
//		m_VTMortality = Mortality;
//		m_VTEnable = true;
//	}
//	else m_VTEnable = false;
//	m_VTTreatmentActive = false;
//}
//
//void CColony::SetMiticideTreatment(CMiteTreatments& theTreatments, BOOL Enable)
//{
//	if (Enable)
//	{
//		m_VTEnable = true;
//		CMiteTreatmentItem DestItem;
//		CMiteTreatmentItem SourceItem;
//		m_MiteTreatmentInfo.ClearAll();
//		for (int i = 0; i < theTreatments.GetCount(); i++)
//		{
//			theTreatments.GetItem(i, SourceItem);
//			DestItem.theStartTime = SourceItem.theStartTime;
//			DestItem.Duration = SourceItem.Duration;
//			DestItem.PctMortality = SourceItem.PctMortality;
//			DestItem.PctResistant = SourceItem.PctResistant;
//			m_MiteTreatmentInfo.AddItem(DestItem);
//		}	    
//	}
//	else m_VTEnable = false;
//	m_VTTreatmentActive = false;    
//}


void CColony::SetSporeTreatment(int StartDayNum, BOOL Enable)
{
	if (Enable)
	{
		m_SPStart = StartDayNum; 
		m_SPEnable = true;
	}
	else m_SPEnable = false;
	m_SPTreatmentActive = false;
}

void CColony::RemoveDroneComb(double pct)
{
	// Simulates the removal of drone comb.  The variable pct is the amount to 
	// be removed

	if (pct > 100) pct = 100.0;
	if (pct < 0) pct = 0.0;

	POSITION pos;

	pos = Deggs.GetHeadPosition();

	CEgg* theEgg;
	while(pos != NULL)
	{
		theEgg = (CEgg*)Deggs.GetNext(pos);
		theEgg->number *= (int)(100.0-pct);
	}

	pos = Dlarv.GetHeadPosition();
	CLarva* theLarv;
	while(pos != NULL)
	{
		theLarv = (CLarva*)Dlarv.GetNext(pos);
		theLarv->number *= (int)(100.0-pct);
	}

	pos = CapDrn.GetHeadPosition();

	CBrood* theBrood;
	while(pos != NULL)
	{
		theBrood = (CBrood*)CapDrn.GetNext(pos);
		theBrood->number *= (int)(100.0-pct);
		theBrood->m_Mites = theBrood->m_Mites * (100.0 - pct);
		theBrood->SetPropVirgins(0.0);
	}
}


void CColony::AddDiscreteEvent(CString datestg, UINT EventID)
{
	CUIntArray* pEventArray = NULL;
	if (m_EventMap.Lookup(datestg,(CObject*&)pEventArray))
	{
		// Date already exists, add a new event to the array
		pEventArray->Add(EventID);
	}
	else
	{
		// Create new map element
		pEventArray = new CUIntArray();
		pEventArray->Add(EventID);
		m_EventMap.SetAt(datestg, pEventArray);
	}
}

void CColony::RemoveDiscreteEvent(CString datestg, UINT EventID)
{
	CUIntArray* pEventArray=NULL;
	if (m_EventMap.Lookup(datestg,(CObject*&)pEventArray))
	{
		// Date exists
		for (int i = 0; i < pEventArray->GetSize(); i++)
		{
			if (pEventArray->GetAt(i) == EventID) pEventArray->RemoveAt(i--);
		}
		if (pEventArray->GetSize() == 0)
		{
			delete pEventArray;
			m_EventMap.RemoveKey(datestg);
		}
	}

}


BOOL CColony::GetDiscreteEvents(CString key, CUIntArray*& theArray)
{
	return m_EventMap.Lookup(key,(CObject*&)theArray);
}

// DoPendingEvents is used when running WebBeePop.  The predefined events from a legacy program are
// mapped into VarroaPop parameters and this is executed as part of the main simulation loop.  A much 
// simplified set of features for use by elementary school students.
void CColony::DoPendingEvents(CEvent* pWeatherEvent, int CurrentSimDay)
{
	CUIntArray* pEventArray = NULL;

	if (!GetDiscreteEvents(pWeatherEvent->GetDateStg("%m/%d/%Y"),pEventArray)) return;
	for (int i=0; i<pEventArray->GetSize(); i++)
	{
		TRACE("A Discrete Event on %s\n",pWeatherEvent->GetDateStg("%m/%d/%Y"));
		//MyMessageBox("Looking For a Discrete Event\n");
		int EggLayDelay = 17;
		int Strength = 5;

		switch(pEventArray->GetAt(i))
		{
		case DE_SWARM:	// Swarm
			//MyMessageBox("Detected SWARM Discrete Event\n");
			AddEventNotification(pWeatherEvent->GetDateStg("%m/%d/%Y"), "Detected SWARM Discrete Event");
			foragers.FactorQuantity(0.75);
			Wadl.FactorQuantity(0.75);
			Dadl.FactorQuantity(0.75);
			break;

		case DE_CHALKBROOD:	// Chalk Brood
			//  All Larvae Die
			AddEventNotification(pWeatherEvent->GetDateStg("%m/%d/%Y"), "Detected CHALKBROOD Discrete Event");
			Dlarv.FactorQuantity(0.0);
			Wlarv.FactorQuantity(0.0);
			break;

		case DE_RESOURCEDEP:	// Resource Depletion
			//  Forager Lifespan = minimum
			AddEventNotification(pWeatherEvent->GetDateStg("%m/%d/%Y"), "Detected RESOURCEDEPLETION Discrete Event");
			m_InitCond.m_ForagerLifespan = 4;
			break;


		case DE_SUPERCEDURE:	// Supercedure of Queen
			//  New queen = 17 days before egg laying starts
			AddEventNotification(pWeatherEvent->GetDateStg("%m/%d/%Y"), "Detected SUPERCEDURE Discrete Event");
			queen.ReQueen(EggLayDelay,Strength,CurrentSimDay);
			break;

		case DE_PESTICIDE:	// Death of foragers by pesticide
			//  25% of foragers die
			AddEventNotification(pWeatherEvent->GetDateStg("%m/%d/%Y"), "Detected PESTICIDE Discrete Event");
			foragers.FactorQuantity(0.75);
			break;

		default:
			break;
		}
	}
}




double CColony::GetMitesDyingToday()
{
	return m_MitesDyingToday;
}

int CColony::GetNurseBees()
// Number of nurse bees is defined as # larvae/2.  Implication is that a nurse bee is needed for each two larvae
{
	int TotalLarvae = Wlarv.GetQuantity() + Dlarv.GetQuantity();
	return TotalLarvae/2;
}

double CColony::GetTotalMiteCount()
{
	return ( RunMite.GetTotal() + CapDrn.GetMiteCount() + CapWkr.GetMiteCount() );

}

void CColony::SetStartSamplePeriod()
{
	// Notifies CColony that it is the beginning of a sample period.  Since we gather either weekly or 
	// daily data this is used to reset accumulators.
	m_MitesDyingThisPeriod = 0;
}

double CColony::GetMitesDyingThisPeriod()
{
	return m_MitesDyingThisPeriod;
}


////////////////////////////////////////////////////////////////////////////////
// Pesticide Dose and Mortality calculations
//
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// ApplyPesticideMortality
//
// Called after ConsumeFood and DetermineFoliarDose.  At that point, the m_EPAData.m_D_ variables
// contain the current exposure of each bee life stage in grams/bee.  This function performs the appropriate reductions
// of bee quantities based on the doses and the appropriate Slope/LD50 for the bee type and current pesticide.
//
// Constraint:  Bee quantities are not reduced unless the current pesticide dose is > previous maximum dose.  But, 
//              for bees just getting into Larva4 or Adult1, this is the first time they have had a dose.
//
//
void CColony::ApplyPesticideMortality()
{
	// Worker Larvae 4
	//if (m_EPAData.m_D_L4 > m_EPAData.m_D_L4_Max) // IED - only reduce if current dose greater than previous maximum dose
	{
		m_DeadWorkerLarvaePesticide = ApplyPesticideToBees(&Wlarv,3,3,m_EPAData.m_D_L4,0,m_EPAData.m_AI_LarvaLD50,m_EPAData.m_AI_LarvaSlope);
		if (m_EPAData.m_D_L4 > m_EPAData.m_D_L4_Max) m_EPAData.m_D_L4_Max = m_EPAData.m_D_L4;
	}

	// Worker Larvae 5
	if (m_EPAData.m_D_L5 > m_EPAData.m_D_L5_Max) 
	{
		m_DeadWorkerLarvaePesticide += ApplyPesticideToBees(&Wlarv,4,4,m_EPAData.m_D_L5,m_EPAData.m_D_L5_Max,m_EPAData.m_AI_LarvaLD50,m_EPAData.m_AI_LarvaSlope);
		m_EPAData.m_D_L5_Max = m_EPAData.m_D_L5;
	}

	// Drone Larvae
	m_DeadDroneLarvaePesticide = ApplyPesticideToBees(&Dlarv,3,3,m_EPAData.m_D_LD,0,m_EPAData.m_AI_LarvaLD50,m_EPAData.m_AI_LarvaSlope); // New L4 drones
	if (m_EPAData.m_D_LD > m_EPAData.m_D_LD_Max)
	{
		m_DeadDroneLarvaePesticide += ApplyPesticideToBees(&Dlarv,4,DLARVLIFE-1,m_EPAData.m_D_LD,m_EPAData.m_D_LD_Max,m_EPAData.m_AI_LarvaLD50,m_EPAData.m_AI_LarvaSlope);
		m_EPAData.m_D_LD_Max = m_EPAData.m_D_LD;

	}


	// Worker Adults 1-3
	m_DeadWorkerAdultsPesticide = ApplyPesticideToBees(&Wadl,0,0,m_EPAData.m_D_A13,0,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope); // New adults
	if (m_EPAData.m_D_A13 > m_EPAData.m_D_A13_Max) 
	{
		m_DeadWorkerAdultsPesticide += ApplyPesticideToBees(&Wadl,1,2,m_EPAData.m_D_A13,m_EPAData.m_D_A13_Max,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope);
		m_EPAData.m_D_A13_Max = m_EPAData.m_D_A13;
	}


	// Worker Adults 4-10
	if (m_EPAData.m_D_A410 > m_EPAData.m_D_A410_Max) 
	{
		m_DeadWorkerAdultsPesticide += ApplyPesticideToBees(&Wadl,3,9,m_EPAData.m_D_A410,m_EPAData.m_D_A410_Max,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope);
		m_EPAData.m_D_A410_Max = m_EPAData.m_D_A410;
	}

	// Worker Adults 11-20
	if (m_EPAData.m_D_A1120 > m_EPAData.m_D_A1120_Max) 
	{
		m_DeadWorkerAdultsPesticide += ApplyPesticideToBees(&Wadl,10,WADLLIFE-1,m_EPAData.m_D_A1120,m_EPAData.m_D_A1120_Max,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope);
		m_EPAData.m_D_A1120_Max = m_EPAData.m_D_A1120;
	}

	// Worker Drones
	m_DeadDroneAdultsPesticide = ApplyPesticideToBees(&Dadl,0,0,m_EPAData.m_D_AD,0,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope);
	if (m_EPAData.m_D_AD > m_EPAData.m_D_AD_Max) 
	{
		m_DeadDroneAdultsPesticide += ApplyPesticideToBees(&Dadl,1,DADLLIFE-1,m_EPAData.m_D_AD,m_EPAData.m_D_AD_Max,m_EPAData.m_AI_AdultLD50,m_EPAData.m_AI_AdultSlope);
		m_EPAData.m_D_AD_Max = m_EPAData.m_D_AD;
	}

	// Foragers - Contact Mortality
	m_DeadForagersPesticide = ApplyPesticideToBees(&foragers,0,0,m_EPAData.m_D_C_Foragers,0,m_EPAData.m_AI_AdultLD50_Contact,m_EPAData.m_AI_AdultSlope_Contact);
	if (m_EPAData.m_D_C_Foragers > m_EPAData.m_D_C_Foragers_Max) 
	{
		m_DeadForagersPesticide += ApplyPesticideToBees(&foragers,1,foragers.GetLength() - 1,m_EPAData.m_D_C_Foragers,m_EPAData.m_D_C_Foragers_Max,m_EPAData.m_AI_AdultLD50_Contact,m_EPAData.m_AI_AdultSlope_Contact);
		m_EPAData.m_D_C_Foragers_Max = m_EPAData.m_D_C_Foragers;
	}

	// Foragers - Diet Mortality
	m_DeadForagersPesticide += ApplyPesticideToBees(&foragers, 0, 0, m_EPAData.m_D_D_Foragers, 0, m_EPAData.m_AI_AdultLD50, m_EPAData.m_AI_AdultSlope);
	if (m_EPAData.m_D_D_Foragers > m_EPAData.m_D_D_Foragers_Max)
	{
		m_DeadForagersPesticide += ApplyPesticideToBees(&foragers, 1, foragers.GetLength() - 1, m_EPAData.m_D_D_Foragers, m_EPAData.m_D_D_Foragers_Max, m_EPAData.m_AI_AdultLD50, m_EPAData.m_AI_AdultSlope);
		m_EPAData.m_D_D_Foragers_Max = m_EPAData.m_D_D_Foragers;
	}
	if (m_DeadForagersPesticide > 0)
	{
		int i = 0;
	}
	// Reset the current doses to zero after mortality is applied.
	m_EPAData.m_D_L4 = 0;
	m_EPAData.m_D_L5 = 0;
	m_EPAData.m_D_LD = 0;
	m_EPAData.m_D_A13 = 0;
	m_EPAData.m_D_A410 = 0;
	m_EPAData.m_D_A1120 = 0;
	m_EPAData.m_D_AD = 0;
	m_EPAData.m_D_C_Foragers = 0;
	m_EPAData.m_D_D_Foragers = 0;
}

// QuantityPesticideToKill
// This just calculates the number of bees in the list that would be killed by the pesticide and dose
int CColony::QuantityPesticideToKill(CBeelist* pList, double CurrentDose, double MaxDose, double LD50, double Slope)
{
		int BeeQuant;
		int NewBeeQuant;
		BeeQuant = pList->GetQuantity();
		double reduxcurrent = m_EPAData.DoseResponse(CurrentDose, LD50, Slope);
		double reduxmax = m_EPAData.DoseResponse(MaxDose, LD50, Slope);
		if (reduxcurrent <= reduxmax) return 0;  // Less than max already seen
		NewBeeQuant = (int)(BeeQuant * ( 1 - (reduxcurrent - reduxmax)));
		//pList->SetQuantityAt(from,to,NewBeeQuant);
		return BeeQuant - NewBeeQuant;  // This is the number killed by pesticide
}

// ApplyPesticideToBees
// This calculates the number of bees to kill then reduces that number from all age groups between "from" and "to" in the list.
int CColony::ApplyPesticideToBees(CBeelist* pList, int from, int to, double CurrentDose, double MaxDose, double LD50, double Slope)
{
		int BeeQuant;
		int NewBeeQuant;
		BeeQuant = pList->GetQuantityAt(from,to);
		if (BeeQuant <= 0) return 0;
		double reduxcurrent = m_EPAData.DoseResponse(CurrentDose, LD50, Slope);
		double reduxmax = m_EPAData.DoseResponse(MaxDose, LD50, Slope);
		if (reduxcurrent <= reduxmax) return 0;  // Less than max already seen
		NewBeeQuant = (int)(BeeQuant * ( 1 - (reduxcurrent - reduxmax)));
		double PropRedux = (double)NewBeeQuant/(double)BeeQuant;
		//pList->SetQuantityAt(from,to,NewBeeQuant);
		pList->SetQuantityAtProportional(static_cast<size_t>(from),static_cast<size_t>(to),PropRedux);
		return BeeQuant - NewBeeQuant;  // This is the number killed by pesticide
}



///////////////////////////////////////////////////////////////////////////////////////
// DetermineFoliarDose
//
// If we are in a date range with Dose, this routing adds to the Dose rate variables.
// 
void CColony::DetermineFoliarDose(int DayNum)
{
	// Jump out if Foliar is not enabled
	if (!m_EPAData.m_FoliarEnabled) return;

	COleDateTime* pDate = GetDayNumDate(DayNum);
	COleDateTime CurDate(pDate->GetYear(), pDate->GetMonth(), pDate->GetDay(), 0,0,0);
	delete pDate;
	
	// In order to expose, must be after the application date and inside the forage window
	if ((CurDate >= m_EPAData.m_FoliarAppDate) && (CurDate >= m_EPAData.m_FoliarForageBegin) && (CurDate < m_EPAData.m_FoliarForageEnd))
	{
		LONG DaysSinceApplication = (CurDate - m_EPAData.m_FoliarAppDate).GetDays();

		// Foliar Dose is related to AI application rate and Contact Exposure factor (See
		// Kris Garber's EFED Training Insect Exposure.pptx for a summary); 

		double Dose = m_EPAData.m_E_AppRate * m_EPAData.m_AI_ContactFactor/1000000.0;  // convert to Grams AI/bee

		// Dose reduced due to active ingredient half-life
		if (m_EPAData.m_AI_HalfLife > 0)
		{
			double k = log(2.0)/m_EPAData.m_AI_HalfLife;
			Dose *= exp(-k*DaysSinceApplication);
		}
		m_EPAData.m_D_C_Foragers += Dose;  // Adds to any diet-based exposure.  Only foragers impacted.
	}

}
/////////////////////////////////////////////////////////////////////////////////
// ConsumeFood 
//
// Determines colony needs, incoming nutrient availability and stored resource availability
// and adjusts the values accordingly.  When this function is complete, the diet-based pesticide Dose (per bee)
// for each lifestage is known and can be used in the mortality calculations.
void CColony::ConsumeFood(CEvent* pEvent, int DayNum)
{
	if (DayNum == 1) return;


	if (IsPollenFeedingDay(pEvent))
	{
		TRACE("Pollen Feeding Day on %s\n",pEvent->GetDateStg("%m/%d/%Y"));
	}
	if (IsNectarFeedingDay(pEvent))
	{
		TRACE("Nectar Feeding Day on %s\n",pEvent->GetDateStg("%m/%d/%Y"));

	}

	double NeedP = GetPollenNeeds(pEvent);	// The amount of pollen the colony needs today
	double NeedN = GetNectarNeeds(pEvent);	// The amount of nectar the colony needs today
	double InP = 0;							// The amount of pollen coming into the colony today
	double InN = 0;							// The amount of nectar coming into the colony today
	double C_AI_P = 0;						// The incoming pollen concentration today in Grams AI/Gram
	double C_AI_N = 0;						// The incoming nectar concentration today in Grams AI/Gram
	double C_Actual_P = 0;					// The actual concentration of exposure the bees receive this day in Grams AI/Gram
	double C_Actual_N = 0;					// The actual concentration of exposure the bees receive this day in Grams AI/Gram

	SResourceItem theResource;

	if (pEvent->IsForageDay())  
	{
		// Pollen
		if (IsPollenFeedingDay(pEvent))
		{
			InP = GetIncomingPollenQuant();
			m_SuppPollen.m_CurrentAmount -= NeedP;  // Decrement current supplemental pollen
			TRACE("Current Supp Pollen = %6.3f\n",m_SuppPollen.m_CurrentAmount);
			NeedP = 0; // All pollen needs are met by paddies and all incoming pollen will be stored - no incoming needed by colony
			C_AI_P = 0;
		}
		else 
		{
			InP = GetIncomingPollenQuant();
			C_AI_P = GetIncomingPollenPesticideConcentration(DayNum);
		}

		// Nectar
		if (IsNectarFeedingDay(pEvent))
		{
			// For nectar feeding day, move supplemental nectar to stores then process same as when no supplemental
			theResource.m_ResourseQuantity = m_SuppNectar.m_StartingAmount / ((m_SuppNectar.m_EndDate - m_SuppNectar.m_BeginDate).GetDays());
			theResource.m_PesticideQuantity = 0;
			AddNectarToResources(theResource);
			//m_Resources.AddNectar(theResource);
			m_SuppNectar.m_CurrentAmount -= theResource.m_ResourseQuantity;  // Decrement current supplemental nectar
			TRACE("Current Supp Nectar = %6.3f\n", m_SuppNectar.m_CurrentAmount);

			InN = GetIncomingNectarQuant();
			C_AI_N = 0;
		}
		else
		{
			InN = GetIncomingNectarQuant();
			C_AI_N = GetIncomingNectarPesticideConcentration(DayNum);
		}
	}
	else // Non foraging day
	{
		InP = 0;
		if (IsPollenFeedingDay(pEvent))
		{
			m_SuppPollen.m_CurrentAmount -= NeedP;  // Decrement current supplemental pollen
			NeedP = 0;  // Pollen needs met by paddies
		}
		InN = 0;
		if (IsNectarFeedingDay(pEvent))
		{
			// For nectar feeding day, move supplemental nectar to stores then process same as when no supplemental
			theResource.m_ResourseQuantity = m_SuppNectar.m_StartingAmount / ((m_SuppNectar.m_EndDate - m_SuppNectar.m_BeginDate).GetDays());
			theResource.m_PesticideQuantity = 0;
			AddNectarToResources(theResource);
		}
		C_AI_P = 0;
		C_AI_N = 0;
	}

	// Check to see if we need to use stored pollen
	if (InP >= NeedP) // Enough incoming pollen to meet need - don't use stores 
	{
		C_Actual_P = C_AI_P;
		theResource.m_ResourseQuantity = InP - NeedP;
		theResource.m_PesticideQuantity = (InP - NeedP) * C_Actual_P;
		AddPollenToResources(theResource);
	}
	else // Less incoming pollen than needed
	{
		double Shortfall = NeedP - InP;
		if (Shortfall < 0) Shortfall = 0;  

		// Calculate resultant concentration [C1*Q1 + C2*Q2]/[Q1 + Q2]
		C_Actual_P = ((C_AI_P * InP) + (m_Resources.GetPollenPesticideConcentration() * Shortfall))/(InP + Shortfall);

		if (m_Resources.GetPollenQuantity() < Shortfall) //&& pEvent->IsWinterDay()) 
		{
			if (m_NoResourceKillsColony)
			{			
				KillColony();
				AddEventNotification(pEvent->GetDateStg("%m/%d/%Y"), "Colony Died - Lack of Pollen Stores");
			}
		}
		//else 
		m_Resources.RemovePollen(Shortfall);
	}

	// Check to see if we need to use stored nectar
	if (InN >= NeedN) // Enough incoming nectar
	{
		C_Actual_N = C_AI_N;
		theResource.m_ResourseQuantity = InN - NeedN;
		theResource.m_PesticideQuantity = (InN - NeedN) * C_Actual_N;
		AddNectarToResources(theResource);
	}
	else // Less incoming nectar than need
	{
		double Shortfall = NeedN - InN;
		if (Shortfall < 0) Shortfall = 0; 

		// Calculate resultant concentration [C1*Q1 + C2*Q2]/[Q1 + Q2]
		C_Actual_N = ((C_AI_N * InN) + (m_Resources.GetNectarPesticideConcentration() * Shortfall))/(InN + Shortfall);

		if (m_Resources.GetNectarQuantity() < Shortfall) // && pEvent->IsWinterDay()) 
		{
			if (m_NoResourceKillsColony)
			{
				KillColony();
				TRACE("Killed Colony due to lack of Nectar stores on %s\n", pEvent->GetDateStg("%m/%d/%Y"));
				AddEventNotification(pEvent->GetDateStg("%m/%d/%Y"), "Colony Died - Lack of Nectar Stores");
			}
		}
		//else 
		m_Resources.RemoveNectar(Shortfall);
		if (m_Resources.GetNectarQuantity() < 0)
		{
			int i = 1;
		}
	}

	m_EPAData.m_D_L4 = C_Actual_P * m_EPAData.m_C_L4_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_L4_Nectar/1000.0;
	m_EPAData.m_D_L5 = C_Actual_P * m_EPAData.m_C_L5_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_L5_Nectar/1000.0;
	m_EPAData.m_D_LD = C_Actual_P * m_EPAData.m_C_LD_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_LD_Nectar/1000.0;
	m_EPAData.m_D_A13 = C_Actual_P * m_EPAData.m_C_A13_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_A13_Nectar/1000.0;
	m_EPAData.m_D_A410 = C_Actual_P * m_EPAData.m_C_A410_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_A410_Nectar/1000.0;
	m_EPAData.m_D_A1120 = C_Actual_P * m_EPAData.m_C_A1120_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_A1120_Nectar/1000.0;
	m_EPAData.m_D_AD = C_Actual_P * m_EPAData.m_C_AD_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_AD_Nectar/1000.0;
	m_EPAData.m_D_D_Foragers = C_Actual_P * m_EPAData.m_C_Forager_Pollen/1000.0 + C_Actual_N * m_EPAData.m_C_Forager_Nectar/1000.0;
}

void CColony::AddPollenToResources(SResourceItem theResource)
{
	// Add pollen but don't exceed the maximum amount
	// If CurrentPollen/MaxPollen > 0.9, Add theResources * (1 - CurrentPollen/MaxPollen)
	if (m_ColonyPolMaxAmount <= 0) 
	{
		m_pSession->AddToInfoList("Maximum Colony Pollen is <= 0.  Forcing to 5000g");
		m_ColonyPolMaxAmount = 5000;
	}
	double PropFull = m_Resources.GetPollenQuantity()/m_ColonyPolMaxAmount;
	double Reduction = 1 - m_Resources.GetPollenQuantity()/m_ColonyPolMaxAmount;
	if (PropFull > 0.9)
	{
		theResource.m_ResourseQuantity *= Reduction;
		theResource.m_PesticideQuantity *= Reduction;
	}

	m_Resources.AddPollen(theResource);
}

void CColony::AddNectarToResources(SResourceItem theResource)
{
	// Add nectar but don't exceed the maximum amount
	if (m_ColonyNecMaxAmount <= 0) 
	{
		m_pSession->AddToInfoList("Maximum Colony Nectar is <= 0.  Forcing to 5000g");
		m_ColonyNecMaxAmount = 5000;
	}
	double PropFull = m_Resources.GetNectarQuantity()/m_ColonyNecMaxAmount;

	double Reduction = 1 - m_Resources.GetNectarQuantity()/m_ColonyNecMaxAmount;
	if (Reduction < 0)
	{
		// Don't exceed Max Value
		Reduction = 0;
	}
	if (PropFull > 0.9)
	{
		theResource.m_ResourseQuantity *= Reduction;
		theResource.m_PesticideQuantity *= Reduction;
	}

	m_Resources.AddNectar(theResource);
}


// Supplemental feeding days are days when:
//   There is some supplemental feed available (current amount > 0),
//   The current day is in the window of Begin Date and End Date
//	 The colony is not dead
//
// Note:  Still need to confirm this.  May eliminate end date logic if 
// the decision is made to just consume the resources and quit.
bool CColony::IsPollenFeedingDay(CEvent* pEvent)
{
	bool FeedingDay = false;

	if ((m_SuppPollenEnabled) && (GetColonySize() > 100))  // Supplemental enabled and colony still alive
	{
		if (m_SuppPollenAnnual)
		{
			
			COleDateTime TestBeginTime(pEvent->GetTime().GetYear(), m_SuppPollen.m_BeginDate.GetMonth(), m_SuppPollen.m_BeginDate.GetDay(),0,0,0);
			COleDateTime TestEndTime(pEvent->GetTime().GetYear(), m_SuppPollen.m_EndDate.GetMonth(), m_SuppPollen.m_EndDate.GetDay(),0,0,0);

			FeedingDay = ((m_SuppPollen.m_CurrentAmount > 0.0) &&
				(TestBeginTime < pEvent->GetTime()) &&
				(TestEndTime >= pEvent->GetTime()));
			
		}
		else
		{
			FeedingDay = ((m_SuppPollen.m_CurrentAmount > 0.0) &&
				(m_SuppPollen.m_BeginDate < pEvent->GetTime()) &&
				(m_SuppPollen.m_EndDate >= pEvent->GetTime()));
		}
	}
	return FeedingDay;
}

bool CColony::IsNectarFeedingDay(CEvent* pEvent)
{
	bool FeedingDay = false;
	if ((m_SuppNectarEnabled) && (GetColonySize() > 100))  // Supplemental enabled and colony still alive
	{
		if (m_SuppNectarAnnual)
		{
			COleDateTime TestBeginTime(pEvent->GetTime().GetYear(), m_SuppNectar.m_BeginDate.GetMonth(), m_SuppNectar.m_BeginDate.GetDay(),0,0,0);
			COleDateTime TestEndTime(pEvent->GetTime().GetYear(), m_SuppNectar.m_EndDate.GetMonth(), m_SuppNectar.m_EndDate.GetDay(),0,0,0);

			FeedingDay = ((m_SuppNectar.m_CurrentAmount > 0.0) &&
				(TestBeginTime < pEvent->GetTime()) &&
				(TestEndTime >= pEvent->GetTime()));		}
		else
		{
			FeedingDay = ((m_SuppNectar.m_CurrentAmount > 0.0) &&
				(m_SuppNectar.m_BeginDate < pEvent->GetTime()) &&
				(m_SuppNectar.m_EndDate >= pEvent->GetTime()));
		}
	}
	return FeedingDay;
}

void CColony::InitializeColonyResources(void)
{
	////////////////////////////////////////////////
	// TODO:
	// This should ultimately be pre-settable at the beginning of a simulation.
	// For now, initialize everything to 0.0.
	m_Resources.SetPollenQuantity(0);
	m_Resources.SetNectarQuantity(0);
	m_Resources.SetPollenPesticideQuantity(0);
	m_Resources.SetNectarPesticideQuantity(0);
}

// returns value in grams.  EPA data for consumption in mg.
double CColony::GetPollenNeeds(CEvent* pEvent)
{
	double Need = 0;

	if (pEvent->IsWinterDay())
	{
		//Need = GetColonySize()*.002; // 2 mg per day per bee
		// Nurse bees have different consumption rates in winter than other adults so need to calculate separately
		// Nurse bees are the youngest of the age groups so determine how many of the youngest bees are nurse bees and
		// compute need based on that
		int WadlAG[3] = {0,0,0};  // Worker Adult Age Groups for consumption rates
		WadlAG[0]= Wadl.GetQuantityAt(0, 2);
		WadlAG[1] = Wadl.GetQuantityAt(3, 9);
		WadlAG[2] = Wadl.GetQuantityAt(10, 19);
		double Consumption[3] = {0,0,0};  // Consumption rates for each of the Adult Age Groups in grams
		Consumption[0] = m_EPAData.m_C_A13_Pollen/1000.0;
		Consumption[1] = m_EPAData.m_C_A410_Pollen/1000.0;
		Consumption[2] = m_EPAData.m_C_A1120_Pollen/1000.0;

		int NurseBeeQuantity = GetNurseBees();
		int MovedNurseBees = 0;

		for (int i = 0; i < 3; i++)
		{
			if (WadlAG[i] <= NurseBeeQuantity - MovedNurseBees)  // This age group has fewer bees than required to fill nurse bee quantity
			{
				MovedNurseBees += WadlAG[i];
				Need += WadlAG[i] * Consumption[i];
			}
			else// This age group has more bees than required to fill nurse bee quantity
			{
				MovedNurseBees += (NurseBeeQuantity - MovedNurseBees);
				Need += (NurseBeeQuantity - MovedNurseBees) * Consumption[i];
			}
			if (MovedNurseBees >= NurseBeeQuantity) 
			{
				break;  // All nurse bees accounted for.
			}
		}
		int NonNurseBees = GetColonySize() - MovedNurseBees;
		Need += NonNurseBees*.002;  // 2 mg per day consumption for non nurse bees

		// Add Forager need
		double ForagerNeed = 0;
		if (pEvent->IsForageDay())
		{
			ForagerNeed = foragers.GetActiveQuantity()*m_EPAData.m_C_Forager_Pollen/1000; //grams
			ForagerNeed += (foragers.GetQuantity() - foragers.GetActiveQuantity())*m_EPAData.m_C_A1120_Pollen/1000; // Unemployed consume like adults
		}
		else
		{
			ForagerNeed = foragers.GetQuantity()*.002;
		}
		Need += ForagerNeed;
	}
	else // non-winter day
	{
		// Larvae needs
		double LNeeds = Wlarv.GetQuantityAt(3)*m_EPAData.m_C_L4_Pollen + Wlarv.GetQuantityAt(4)*m_EPAData.m_C_L5_Pollen + 
			Dlarv.GetQuantity()*m_EPAData.m_C_LD_Pollen;

		// Adult needs
		double ANeeds;
		if (pEvent->IsForageDay())
		{
			ANeeds = Wadl.GetQuantityAt(0,2)*m_EPAData.m_C_A13_Pollen + Wadl.GetQuantityAt(3,9)*m_EPAData.m_C_A410_Pollen +
				Wadl.GetQuantityAt(10,19)*m_EPAData.m_C_A1120_Pollen + Dadl.GetQuantity()*m_EPAData.m_C_AD_Pollen + 
				foragers.GetActiveQuantity()*m_EPAData.m_C_Forager_Pollen + foragers.GetUnemployedQuantity()*m_EPAData.m_C_A1120_Pollen;
				// Unemployed foragers consume like most mature adult house bees
		}
		else // All foragers consume the same quantity as the most mature adult house bees on non-forage days
		{
			ANeeds = Wadl.GetQuantityAt(0,2)*m_EPAData.m_C_A13_Pollen + Wadl.GetQuantityAt(3,9)*m_EPAData.m_C_A410_Pollen +
				(Wadl.GetQuantityAt(10,19) + foragers.GetQuantity())*m_EPAData.m_C_A1120_Pollen + Dadl.GetQuantity()*m_EPAData.m_C_AD_Pollen;
		}

		Need = (LNeeds + ANeeds)/1000.0;  // Convert to grams
	}

	
	if (Need > 1000)
	{
		CString stg = pEvent->GetDateStg("%m/%d/%Y");
		int i = 1;
	}
	return Need;
}

// returns value in grams
double CColony::GetNectarNeeds(CEvent* pEvent)
{
	double Need = 0;

	if (pEvent->IsWinterDay())
	{
		if (GetColonySize() > 0)
		{
			if (pEvent->GetTemp() <= 8.5) // See K. Garber's Winter Failure logic
			{
				Need = 0.3121*GetColonySize()*pow(0.128*GetColonySize(), -0.48); //grams
			}
			else  //  8.5 < AveTemp < 18.0
			{
				if (pEvent->IsForageDay())  // In this case, foragers need normal forager nutrition
				{
					double NonForagers = GetColonySize() - foragers.GetActiveQuantity();
					Need = (foragers.GetActiveQuantity()*m_EPAData.m_C_Forager_Nectar)/1000 + 0.05419*NonForagers*pow(0.128*NonForagers, -0.27); //grams
				}
				else  // Otherwise, all bees consume at winter rates
				{
					Need = 0.05419*GetColonySize()*pow(0.128*GetColonySize(), -0.27); //grams
				}
			}
		}
	}
	else // Summer Day
	{

		// Larvae needs
		double LNeeds = Wlarv.GetQuantityAt(3)*m_EPAData.m_C_L4_Nectar + Wlarv.GetQuantityAt(4)*m_EPAData.m_C_L5_Nectar +
			Dlarv.GetQuantity()*m_EPAData.m_C_LD_Nectar;

		// Adult needs
		double ANeeds;
		if (pEvent->IsForageDay())
		{
			ANeeds = Wadl.GetQuantityAt(0,2)*m_EPAData.m_C_A13_Nectar + Wadl.GetQuantityAt(3,9)*m_EPAData.m_C_A410_Nectar +
				Wadl.GetQuantityAt(10,19)*m_EPAData.m_C_A1120_Nectar + foragers.GetUnemployedQuantity()*m_EPAData.m_C_A1120_Nectar + 
				foragers.GetActiveQuantity()*m_EPAData.m_C_Forager_Nectar + Dadl.GetQuantity()*m_EPAData.m_C_AD_Nectar;
		}
		else // Foragers consume the same quantity as the most mature adult house bees
		{
			ANeeds = Wadl.GetQuantityAt(0,2)*m_EPAData.m_C_A13_Nectar + Wadl.GetQuantityAt(3,9)*m_EPAData.m_C_A410_Nectar +
				(Wadl.GetQuantityAt(10,19) + foragers.GetQuantity())*m_EPAData.m_C_A1120_Nectar + Dadl.GetQuantity()*m_EPAData.m_C_AD_Nectar;
		}

		Need = (LNeeds + ANeeds)/1000.0;  //Convert to grams
	}

	return Need;
}

// returns value in grams.  m_I_NectarLoad is in mg/bee
double CColony::GetIncomingNectarQuant(void)
{
	double Nectar = 0;
	Nectar =  foragers.GetActiveQuantity()  * m_EPAData.m_I_NectarTrips * m_EPAData.m_I_NectarLoad/1000.0;

	// If there are no larvae, all pollen foraging trups will become nectar foraging trips
	if ((Wlarv.GetQuantity() + Dlarv.GetQuantity()) <= 0)
	{
		Nectar +=  foragers.GetActiveQuantity()  * m_EPAData.m_I_PollenTrips * m_EPAData.m_I_NectarLoad/1000.0;	
	}
	return Nectar;
}

// returns value in grams  m_I_PollenLoad is in mg/bee
double CColony::GetIncomingPollenQuant(void)
{
	double Pollen;
	// If there are no larvae, no pollen will be brought in to the colony
	if ((Wlarv.GetQuantity() + Dlarv.GetQuantity()) > 0)
	{
		Pollen = foragers.GetActiveQuantity()  * m_EPAData.m_I_PollenTrips * m_EPAData.m_I_PollenLoad/1000.0;
	}
	else Pollen = 0;
	return Pollen;
}

/////////////////////////////////////////////////////////////////////////////////////////
// The Incoming Pesticide Concentration functions are responsible to determine
// if we are in a window where pesticide present as well as calculating any decay of 
// effectiveness (effectiveness decay is represented by a reduced "effective" concentration
//
// Returns Gram AI/Gram
double CColony::GetIncomingNectarPesticideConcentration(int DayNum)
{
	double IncomingConcentration = 0;
	COleDateTime* pDate = GetDayNumDate(DayNum);
	COleDateTime CurDate(pDate->GetYear(), pDate->GetMonth(), pDate->GetDay(), 0,0,0);
	delete pDate;

	// Check to see if we're using only pesticide concentration table
	// If so, query this date and set Nectar pesticide concentration as indicated in the file
	//
	//
	// if not, continue
	if (m_NutrientCT.IsEnabled())
	{
		double NectConc, PolConc;
		m_NutrientCT.GetContaminantConc(CurDate,NectConc, PolConc);
		if (NectConc > 0.0)
		{
			int i = 0;
		}
		IncomingConcentration = NectConc;
	}
	else  // Normal process
	{
		if ((m_EPAData.m_FoliarEnabled) && (CurDate >= m_EPAData.m_FoliarAppDate) && (CurDate >= m_EPAData.m_FoliarForageBegin) && (CurDate < m_EPAData.m_FoliarForageEnd))
		{
			IncomingConcentration = 110.0 * m_EPAData.m_E_AppRate/1000000.0;  // Incoming AI Grams/Grams Nectar due to foliar spray
			// Dose reduced due to active ingredient half-life
			LONG DaysSinceApplication = (CurDate - m_EPAData.m_FoliarAppDate).GetDays();
			if (m_EPAData.m_AI_HalfLife > 0)
			{
				double k = log(2.0)/m_EPAData.m_AI_HalfLife;
				IncomingConcentration *= exp(-k*DaysSinceApplication);
			}
			AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Foliar Spray Nectar Pesticide");
		}
		if ((CurDate >= m_EPAData.m_SeedForageBegin) && (CurDate < m_EPAData.m_SeedForageEnd) && (m_EPAData.m_SeedEnabled))
		{
			IncomingConcentration += m_EPAData.m_E_SeedConcentration/1000000.0; // Add Incoming Grams AI/Grams Bectar due to seed concentration
			AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Seed Nectar Pesticide");
		}
		if ((CurDate >= m_EPAData.m_SoilForageBegin) && (CurDate < m_EPAData.m_SoilForageEnd) && (m_EPAData.m_SoilEnabled)) // In soil forage range
		{
			if ((m_EPAData.m_AI_KOW > 0) || (m_EPAData.m_E_SoilTheta != 0)) // Ensure no NAN exceptions
			{
				double LogKOW = log10(m_EPAData.m_AI_KOW);
				double TSCF = -0.0648*(LogKOW*LogKOW) + 0.241*LogKOW + 0.5822;
				double SoilConc = TSCF * (pow(10,(0.95*LogKOW -2.05)) + 0.82) *
					m_EPAData.m_E_SoilConcentration*(m_EPAData.m_E_SoilP/(m_EPAData.m_E_SoilTheta + m_EPAData.m_E_SoilP*m_EPAData.m_AI_KOC*m_EPAData.m_E_SoilFoc));
				IncomingConcentration += SoilConc/1000000; // Add Grams AI/Gram Nectar for soil concentration
			AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Soil Nectar Pesticide");
			}
	}
	}
	return IncomingConcentration;  // Grams AI/Gram
}

// Returns Grams AI/Gram
double CColony::GetIncomingPollenPesticideConcentration(int DayNum)
{
	// NOTE: This code is identical to GetIncomingNectarPesticideConcentration

	double IncomingConcentration = 0;
	COleDateTime* pDate = GetDayNumDate(DayNum);
	COleDateTime CurDate(pDate->GetYear(), pDate->GetMonth(), pDate->GetDay(), 0,0,0);
	delete pDate;
	// Check to see if we're using only pesticide concentration table
	// If so, query this date and set Nectar pesticide concentration as indicated in the file
	//
	//
	// if not, continue
	if (m_NutrientCT.IsEnabled())
	{
		double NectConc, PolConc;
		m_NutrientCT.GetContaminantConc(CurDate,NectConc, PolConc);
		IncomingConcentration = PolConc;
	}
	else  // Normal process
	{
		if ((m_EPAData.m_FoliarEnabled) && (CurDate >= m_EPAData.m_FoliarAppDate) && (CurDate >= m_EPAData.m_FoliarForageBegin) && (CurDate < m_EPAData.m_FoliarForageEnd))
		{
			IncomingConcentration = 110.0 * m_EPAData.m_E_AppRate/1000000;  // Incoming AI Grams/Grams Pollen due to foliar spray (uG/G = 110  * lbs/AC)
			// Dose reduced due to active ingredient half-life
			LONG DaysSinceApplication = (CurDate - m_EPAData.m_FoliarAppDate).GetDays();
			if (m_EPAData.m_AI_HalfLife > 0)
			{
				double k = log(2.0)/m_EPAData.m_AI_HalfLife;
				IncomingConcentration *= exp(-k*DaysSinceApplication);
			}
			AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Foliar Spray Pollen Pesticide");
		}
		if ((CurDate >= m_EPAData.m_SeedForageBegin) && (CurDate < m_EPAData.m_SeedForageEnd) && (m_EPAData.m_SeedEnabled))
		{
			IncomingConcentration += m_EPAData.m_E_SeedConcentration/1000000.0; // Add Grams AI/Grams Pollen
			AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Seed Pollen Pesticide");

		}
		if ((CurDate >= m_EPAData.m_SoilForageBegin) && (CurDate < m_EPAData.m_SoilForageEnd) && (m_EPAData.m_SoilEnabled)) // In soil forage range
		{
			if ((m_EPAData.m_AI_KOW > 0) || (m_EPAData.m_E_SoilTheta != 0)) // Ensure no NAN exceptions
			{
				double LogKOW = log10(m_EPAData.m_AI_KOW);
				double TSCF = -0.0648*(LogKOW*LogKOW) + 0.241*LogKOW + 0.5822;
				double SoilConc = TSCF * (pow(10,(0.95*LogKOW -2.05)) + 0.82) *
					m_EPAData.m_E_SoilConcentration*(m_EPAData.m_E_SoilP/(m_EPAData.m_E_SoilTheta + m_EPAData.m_E_SoilP*m_EPAData.m_AI_KOC*m_EPAData.m_E_SoilFoc));
				IncomingConcentration += SoilConc/1000000;  // Add Grams AI/Grams Pollen
				AddEventNotification(CurDate.Format("%m/%d/%Y"), "Incoming Soil Pollen Pesticide");
			}
		}
	}
	return IncomingConcentration; // Grams AI/Gram
}
