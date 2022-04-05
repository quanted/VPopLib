#include "cstring.h"

#include <cassert>
#include <limits>


CString::CString() : m_data()
{
}

CString::CString(const std::string& str) : m_data(str)
{
}

CString::CString(const char* cStr) : m_data(cStr)
{
}

bool CString::operator==(const CString& str) const
{
	return m_data == str.m_data;
}

bool CString::operator==(const char* str) const
{
	return m_data == str;
}

bool CString::operator!=(const CString& str) const
{
	return m_data != str.m_data;
}

bool CString::operator!=(const char* str) const
{
	return m_data != str;
}

char& CString::operator[](const size_t& index)
{
	return m_data[index];
}

const char& CString::operator[](const size_t& index) const
{
	return m_data[index];
}

CString& CString::operator+=(const CString& str)
{
	if (&str != this)
	{
		m_data += str.m_data;
	}
	return *this;
}

CString& CString::operator=(const CString& str)
{
	if (&str != this)
	{
		m_data = str.m_data;
	}
	return *this;
}

CString& CString::operator+=(const char& c)
{
	m_data += c;
	return *this;
}

bool CString::operator<(const CString& str) const
{
	return m_data < str.m_data;
}

const std::string& CString::ToString() const
{
	return m_data;
}

CString::operator const char*() const
{
	return m_data.c_str();
}

int CString::GetLength() const
{
	return static_cast<int>(m_data.length());
}

CString& CString::MakeLower()
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), ::tolower);
	return *this;
}

CString& CString::MakeUpper()
{
	std::transform(m_data.begin(), m_data.end(), m_data.begin(), ::toupper);
	return *this;
}

void CString::Trim()
{
	m_data.erase(m_data.find_last_not_of(whitespace) + 1);
	m_data.erase(0, m_data.find_first_not_of(whitespace));
}

void CString::TrimLeft()
{
	m_data.erase(0, m_data.find_first_not_of(whitespace));
}

void CString::TrimRight()
{
	//ba::trim_right(m_data);  //Trying to get rid of boost here
	m_data.erase(m_data.find_last_not_of(whitespace) + 1);
}

int CString::Find(char element) const
{
	size_t position = m_data.find(element);
	return (position != std::string::npos)? static_cast<int>(position) : -1;
}

int CString::ReverseFind(char element) const
{
	size_t position = m_data.rfind(element);
	return (position != std::string::npos)? static_cast<int>(position) : -1;
}

int CString::Find(const char* str) const
{
	size_t position = m_data.find(str);
	return (position != std::string::npos)? static_cast<int>(position) : -1;
}

int CString::ReverseFind(const char* str) const
{
	size_t position = m_data.rfind(str);
	return (position != std::string::npos)? static_cast<int>(position) : -1;
}

void CString::Replace(const CString& toReplace, const CString& with)
{
	std::string search = toReplace.ToString();
	std::string format = with.ToString();
	size_t pos = m_data.find(search);
	if (pos != std::string::npos)  // Substring found
	{
		size_t len = search.size();
		m_data.replace(pos, len, format);
	}
}

CString CString::Left(int count) const
{
    // make sure that during size_t to int conversion we don't loose data
	assert(std::numeric_limits<int>::max() > m_data.length());
    auto length = static_cast<int>(m_data.length());

	count = std::clamp(count, 0, length);
	return m_data.substr(0, count);
}

CString CString::Right(int count) const
{
    // make sure that during size_t to int conversion we don't loose data
    assert(std::numeric_limits<int>::max() > m_data.length());
    auto length = static_cast<int>(m_data.length());

	count = std::clamp(count, 0, length);
	return m_data.substr(length - count);
}

CString CString::Mid(int first) const
{
    // make sure that during size_t to int conversion we don't loose data
    assert(std::numeric_limits<int>::max() > m_data.length());
    auto length = static_cast<int>(m_data.length());

	first = std::clamp(first, 0, length);
	return m_data.substr(first);
}

CString CString::Mid(int first, int count) const
{
    // make sure that during size_t to int conversion we don't loose data
    assert(std::numeric_limits<int>::max() > m_data.length());
    auto length = static_cast<int>(m_data.length());

	first = std::clamp(first, 0, length);
	count = std::clamp(count, 0, length);
	return m_data.substr(first, count);
}

CString CString::Tokenize(const char* delimiter, int& startPosition) const
{
	CString cResult = "";
	std::string result = "";
	if (startPosition >= 0)
	{
		size_t P1 = m_data.find_first_not_of(delimiter, startPosition);
		size_t P2 = m_data.find_first_of(delimiter, P1);
		if (P1 != -1) result = m_data.substr(P1, P2 - P1);
		startPosition = static_cast<int>(P2);
		cResult = result;
	}
    return cResult;
}


CString operator+(const CString& str1, const CString& str2)
{
    CString cStr1(str1);
	cStr1 += str2;
    return cStr1;
}

CString operator+(const CString& str1, const char* str2)
{
    CString cStr1(str1);
	cStr1 += str2;
    return cStr1;
}

CString operator+(const char* str1, const CString& str2)
{
    CString cStr1(str1);
	cStr1 += str2;
    return cStr1;
}
