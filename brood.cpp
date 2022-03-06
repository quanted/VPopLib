// Brood.cpp: implementation of the CBrood class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "VarroaPop.h"
#include "brood.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBrood::CBrood()
{
	m_Mites = 0;
}

CBrood::~CBrood()
{
}

void CBrood::Reset()
{
	CBee::Reset();
	m_Mites = 0;
}
