// Forager.cpp: implementation of the CAdult class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "VarroaPop.h"
#include "adult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAdult::CAdult()
{
	m_Lifespan = 0.0;
	m_CurrentAge = 0.0;
	m_Mites = 0;
	m_PropVirgins = 0.0;
	m_ForageInc = 0.0;
	number = 0;
	m_MitesCounted = false;
}

CAdult::CAdult(int theNumber)
{
	m_Lifespan = 0.0;
	m_CurrentAge = 0.0;
	m_Mites = 0;
	m_PropVirgins = 0.0;
	m_ForageInc = 0.0;
	number = theNumber;
	m_MitesCounted = false;

}


CAdult::~CAdult()
{
}

void CAdult::Reset()
{
	CBee::Reset();
	m_Lifespan = 0;
	m_CurrentAge = 0;
	m_Mites = 0;
	m_PropVirgins = 0.0;
	m_ForageInc = 0.0;
	number = 0;
	m_MitesCounted = false;

}

CAdult CAdult::operator = (CAdult& adult)
{
	CAdult m;
	m.m_Lifespan = adult.m_Lifespan;
	m.m_CurrentAge = adult.m_CurrentAge;
	m.m_Mites = adult.m_Mites;
	m.m_PropVirgins = adult.m_PropVirgins;
	m.m_ForageInc = adult.m_ForageInc;
	m.number = adult.number;
	m.Alive = adult.Alive;
	m.m_MitesCounted = adult.m_MitesCounted;
	return m;

}

void CAdult::SetPropVirgins(double prop)
{
	if (prop < 0.0) m_PropVirgins = 0.0;
	else if (prop > 1.0) m_PropVirgins = 1.0;
	else m_PropVirgins = prop;
}

