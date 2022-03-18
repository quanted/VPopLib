// Adult.h: interface for the CAdult class.
//
//////////////////////////////////////////////////////////////////////

// For some reason the guard in Adult.h AFX_ADULT_H__8C6C41B6_7899_11D2_8D9A_0020AF233A70__INCLUDED_ 
// was conflicting on some Linux machines so let's get rid of it
#pragma once

#include "bee.h"
#include "mite.h"

class CAdult : public CBee  {
  private:
	float m_Lifespan;
	float m_CurrentAge;
	CMite m_Mites;
	double m_Virgins;
	bool m_MitesCounted;
	double m_ForageInc;

  public:
	CAdult();
	CAdult(int Num);
	CAdult operator = (CAdult& adult);
	void SetLifespan(int span) {m_Lifespan = (float)span;}
	void SetCurrentAge(float age) {m_CurrentAge = age;}
	void SetPropVirgins(double prop) {m_Virgins = prop;}
	double GetPropVirgins() {return m_Virgins;}
	void IncrementAge(float increment) {m_CurrentAge += increment;}
	float GetCurrentAge() {return m_CurrentAge;}
	int GetLifespan() {return int(m_Lifespan);}
	void SetForageInc( double Inc ) {m_ForageInc = Inc;}
	double GetForageInc() {return m_ForageInc;}
	virtual ~CAdult();

	void SetMites(CMite theMites) {m_Mites = theMites;}
	CMite GetMites() {return m_Mites;}
	bool HaveMitesBeenCounted() { return m_MitesCounted; }
	void SetMitesCounted(bool value) { m_MitesCounted = value; }
	void Reset();

};
