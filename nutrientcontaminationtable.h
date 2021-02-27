#pragma once
//#include "afx.h"
#include "coledatetime.h"
#include "cobject.h"
#include "cstring.h"
#include "carray.h"


struct SNCElement 
{
	COleDateTime m_NCDate;
	double m_NCPollenCont;
	double m_NCNectarCont;
};

class CNutrientContaminationTable :
	public CObject
{
private:
public:
	CArray <SNCElement, SNCElement> m_ContDateArray;

public:
	CNutrientContaminationTable(void);
	~CNutrientContaminationTable(void);
	bool LoadTable(CString FilePath);
	void RemoveAll(void) {m_ContDateArray.RemoveAll();}
	void GetContaminantConc(COleDateTime Date, double &NecConc, double &PolConc);
	void AddContaminantConc(SNCElement theElement);
	void operator = (const CNutrientContaminationTable &CNTable);
	CString GetFileName() {return m_ContaminantFileName;}
	BOOL m_NutrientContEnabled;
	CString m_ContaminantFileName;
	BOOL IsEnabled() {return m_NutrientContEnabled;}

};
