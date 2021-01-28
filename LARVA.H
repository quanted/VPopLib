// Larva.h: interface for the CLarva class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LARVA_H__8C6C41B7_7899_11D2_8D9A_0020AF233A70__INCLUDED_)
#define AFX_LARVA_H__8C6C41B7_7899_11D2_8D9A_0020AF233A70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "bee.h"

class CLarva : public CBee  {
  private:
	bool infested = false;
	bool fertilized = false;
	int mites = 0;;
	float infestProbability = 0.0;


  public:
	CLarva();
//	CLarva(int quantity, bool isFertilized=false,
//		   int initialMites=0, float infestProb=1.0);
	CLarva(int quantity) {number = quantity;}
	//CLarva(CLarva* oldLarva);
	CLarva operator = (CLarva& larva);
	//void Serialize(CArchive &ar);
	virtual ~CLarva();
	void Reset();
	//CLarva& operator=(const CLarva& theLarva);
};

#endif // !defined(AFX_LARVA_H__8C6C41B7_7899_11D2_8D9A_0020AF233A70__INCLUDED_)
