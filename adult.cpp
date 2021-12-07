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
	m_Virgins = 0.0;
	m_ForageInc = 0.0;
	number = 0;
}

CAdult::CAdult(int theNumber)
{
	m_Lifespan = 0.0;
	m_CurrentAge = 0.0;
	m_Mites = 0;
	m_Virgins = 0.0;
	m_ForageInc = 0.0;
	number = theNumber;
}


CAdult::~CAdult()
{
}

void CAdult::Reset()
{
	m_Lifespan = 0;
	m_CurrentAge = 0;
	m_Mites = 0;
	m_Virgins = 0.0;
	m_ForageInc = 0.0;
	number = 0;
}

CAdult CAdult::operator = (CAdult& adult)
{
	CAdult m;
	m.m_Lifespan = adult.m_Lifespan;
	m.m_CurrentAge = adult.m_CurrentAge;
	m.m_Mites = adult.m_Mites;
	m.m_Virgins = adult.m_Virgins;
	m.m_ForageInc = adult.m_ForageInc;
	m.number = adult.number;
	return m;

}
