// Mite.cpp: implementation of the CMite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "VarroaPop.h"
#include "mite.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMite::CMite()
{
	m_Resistant = 0;
	m_NonResistant = 0;

}

CMite::CMite(double Res, double NonRes)
{
	m_Resistant = Res;
	m_NonResistant = NonRes;
}

CMite::CMite(const CMite& mite)
{
	m_Resistant = mite.m_Resistant;
	m_NonResistant = mite.m_NonResistant;
}

CMite::~CMite()
{

}

void CMite::Zero()
{
	SetNonResistant(0);
	SetResistant(0);
}


CMite& CMite::operator=(const CMite& mite)
{
	if (this != &mite)
	{
		m_Resistant = mite.m_Resistant;
		m_NonResistant = mite.m_NonResistant;
	}
	return *this;
}

void CMite::SetPctResistant(double pct)
{
	//  Sets the current percent of resistance to pct percent.
	//  Does not change the total number of mites
	ASSERT((pct >=0) && (pct <= 100));  // Percent between 0 and 100?
	double total = m_Resistant+m_NonResistant;
	m_Resistant = total*pct/100;
	m_NonResistant = total - m_Resistant;
//	SetResistant(int(total*pct/float(100)));
//	SetNonResistant(total - GetResistant());
}

// Overloaded operators
//

void CMite::operator+=(CMite theMite)
{
	m_Resistant += theMite.m_Resistant;
	m_NonResistant += theMite.m_NonResistant;
}

void CMite::operator +=(double value)
{
	double pctres = 0;
	if ((m_Resistant + m_NonResistant) > 0) 
		pctres = m_Resistant/(m_Resistant+m_NonResistant);
	double addtores = value*pctres;
	m_Resistant = m_Resistant + addtores;
	m_NonResistant = m_NonResistant + (value - addtores);
}


void CMite::operator -= (CMite theMite)
{
	m_Resistant -= theMite.m_Resistant;
	m_NonResistant -= theMite.m_NonResistant;
}

void CMite::operator -= (double value)
{
	double pctres = 0;
	if (GetTotal() > 0) pctres = m_Resistant * 100 / (m_Resistant + m_NonResistant);
	double subfromres = value*pctres/100.0;
	m_Resistant = m_Resistant - subfromres;
	m_NonResistant = m_NonResistant - (value - subfromres);
	if (m_Resistant < 0) m_Resistant = 0;
	if (m_NonResistant < 0) m_NonResistant = 0;
}

CMite CMite::operator+(CMite theMite)
{
	double res = m_Resistant + theMite.m_Resistant;
	double nres = m_NonResistant + theMite.m_NonResistant;
	return CMite(int(res), int(nres));
}

CMite CMite::operator-(CMite theMite)
{
	double res = m_Resistant - theMite.m_Resistant;
	double nres = m_NonResistant - theMite.m_NonResistant;
	if (res < 0) res = 0;
	if (nres < 0) nres = 0;
	return CMite(res, nres);
}

CMite::operator int()
{
	return int(m_Resistant + m_NonResistant);
}


CMite CMite::operator=(double value)
{
	// An integer assignment will set the NonResistant quantity of mites the the 
	// integer value and set the Resistant quantity to 0
	m_Resistant = 0;
	m_NonResistant = double(value);
	return CMite(m_Resistant, m_NonResistant);
}


CMite CMite::operator=(CMite& theMite)
{
	m_Resistant = theMite.m_Resistant;
	m_NonResistant = theMite.m_NonResistant;
	return CMite(m_Resistant, m_NonResistant);
}




CMite CMite::operator *(double value)
{
	double res = m_Resistant*value;
	double nonres = m_NonResistant*value;
	return CMite(res,nonres);

}

