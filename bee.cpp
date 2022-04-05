// Bee.cpp : implementation file
//

#include "stdafx.h"
//#include "VarroaPop.h"
#include "bee.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBee

IMPLEMENT_DYNCREATE(CBee, CCmdTarget)

CBee::CBee()
{
	number = 0;
	age = 0.0;
	Alive = true;
}

CBee::~CBee()
{
}

void CBee::Reset()
{
	number = 0;
	age = 0.0;
	Alive = true;
}

CBee CBee::operator = (CBee& bee)
{
	CBee temp;
	temp.Alive = bee.Alive;
	temp.number = bee.number;
	return temp;
}

CBee::CBee(int Num)
{
	number = Num;
	age = 0;
	Alive = true;
}

CBee::CBee(CBee& bee)
{
	Alive = bee.Alive;
	number = bee.number;
	age = bee.age;
}



//BEGIN_MESSAGE_MAP(CBee, CCmdTarget)
//	//{{AFX_MSG_MAP(CBee)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

