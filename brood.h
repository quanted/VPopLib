// Brood.h: interface for the CBrood class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BROOD_H__5D99D1A4_DDD5_11D2_8D9A_0020AF233A70__INCLUDED_)
#define AFX_BROOD_H__5D99D1A4_DDD5_11D2_8D9A_0020AF233A70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "bee.h"
#include "mite.h"

class CBrood : public CBee  
{
public:
	CMite m_Mites;			// Mites infesting this group
	double m_PropVirgins;   // Proportion of infesting mites that
							// have never infested before

public:
	CBrood();
	CBrood(int Num) {number = Num; m_Mites = 0;}
	virtual ~CBrood();
	void Reset();

};

#endif // !defined(AFX_BROOD_H__5D99D1A4_DDD5_11D2_8D9A_0020AF233A70__INCLUDED_)
