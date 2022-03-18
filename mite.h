// Mite.h: interface for the CMite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MITE_H__8B31770F_E1C2_497D_87E8_5B55D593932D__INCLUDED_)
#define AFX_MITE_H__8B31770F_E1C2_497D_87E8_5B55D593932D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMite : public CObject  
{
	
protected:
	double m_Resistant;	// Number resistant to miticide
	double m_NonResistant; // Number susceptible to miticide;

public:
	CMite();
	CMite(double Res, double NonRes);
	CMite(const CMite& mite);	// Copy Constructor
	virtual ~CMite();

	CMite& operator=(const CMite& mite);

	double GetResistant() {return m_Resistant;}
	double GetNonResistant() {return m_NonResistant;}
	void SetResistant(double num) {m_Resistant = num;}
	void SetNonResistant(double num) {m_NonResistant = num;}
	void Zero();

	double GetTotal() {return (m_Resistant + m_NonResistant);}
	double GetPctResistant() {return (GetTotal()>0) ? 
						(100.0*m_Resistant/(m_Resistant+m_NonResistant)) : 0;}
	void SetPctResistant(double pct);

	void operator += (CMite mite);
	void operator -= (CMite mite);
	void operator += (double value);
	void operator -= (double value);
	CMite operator + (CMite mite);
	CMite operator - (CMite mite);
	CMite operator = (double value); // Assign quantity to a CMite
	CMite operator = (CMite& theMite);
	CMite operator * (double value);
	//CMite operator * (double value);
	operator int();

};

#endif // !defined(AFX_MITE_H__8B31770F_E1C2_497D_87E8_5B55D593932D__INCLUDED_)
