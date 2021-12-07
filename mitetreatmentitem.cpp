#include "stdafx.h"
#include "coledatetime.h"
#include "mitetreatmentitem.h"

CMiteTreatmentItem::CMiteTreatmentItem(void)
{
}

CMiteTreatmentItem::~CMiteTreatmentItem(void)
{
}


bool CMiteTreatmentItem::IsValid()
{
	bool valid = false;
	valid = (theStartTime.GetStatus() == COleDateTime::valid);
	

	return valid;
}
