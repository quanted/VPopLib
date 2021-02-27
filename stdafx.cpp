#include "stdafx.h"
#include "cstring.h"

std::ostream &operator<<(std::ostream &stream, const CString& string)
{
    return stream << string.ToString();
}
