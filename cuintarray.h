#pragma once
#ifndef CUINTARRAY_CUSTOM_H
#define CUINTARRAY_CUSTOM_H

#include "cobject.h"

#include <cstdint>
#include <vector>

/**
 * Only supports the necessary interface for the good behavior of VarroaPop
 */
class NO_LIBEXPORT CUIntArray : public CObject
{
public:
    //INT_PTR GetSize() const;
    size_t GetSize() const;
    UINT GetAt(INT_PTR index) const;

    void Add(UINT eventId);
    void RemoveAt(UINT index);

	void RemoveAll();

protected:

    std::vector<UINT> m_data;
};

#endif // CUINTARRAY_CUSTOM_H
