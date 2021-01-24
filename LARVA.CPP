// Larva.cpp: implementation of the CLarva class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "VarroaPop.h"
#include "Larva.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLarva::CLarva()
{
}

CLarva::~CLarva()
{
}

void CLarva::Reset()
{
	infested = false;
	fertilized = false;
	mites = 0;
	infestProbability = 0.0;

}


CLarva CLarva::operator = (CLarva& larva)
{
	CLarva temp;
	temp.Alive = larva.Alive;
	temp.number = larva.number;
	temp.infested = larva.infested;
	temp.fertilized = larva.fertilized;
	temp.mites = larva.mites;
	temp.infestProbability = larva.infestProbability;
	return temp;
}