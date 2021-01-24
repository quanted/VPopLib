#pragma once
#include "fmt/core.h"
#include "cstring.h"



template<typename... Args>
void CString::Format(const char* format, Args... args)
{
	m_data = fmt::sprintf(format, std::forward< Args >(args)...);
}
