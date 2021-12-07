#pragma once
//#include "afx.h"
#include "coledatetime.h"

class CMiteTreatmentItem :
    public CObject
{
public:

    COleDateTime theStartTime;
    UINT Duration;
    double PctMortality; // NOTE: Need to change logic in rest of program to treat this like a double (percentage)
    double PctResistant;
	bool IsValid();

    CMiteTreatmentItem(void);
    ~CMiteTreatmentItem(void);
};
