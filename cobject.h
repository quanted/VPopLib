#pragma once
#include "stdafx.h"
#ifndef COBJECT_CUSTOM_H
#define COBJECT_CUSTOM_H

//class CArchive;

/**
 * Only supports the necessary interface for the good behavior of VarroaPop
 */
class NO_LIBEXPORT CObject
{
protected:
	int m_ObInt;

public:
	CObject();
	virtual ~CObject();

};

#endif // COBJECT_CUSTOM_H
