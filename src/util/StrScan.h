/**
 * @file StrScan.h
 * @brief Defines StrScan Class, a part of tf core
 * @author David Shang
 * @version 1.00
 * $Id: $
 */

#pragma once
#include "Platform.h"
#include <stddef.h>
#include <cstring>
#include <string>
#include <vector>

namespace tf {
/**
 * \class StrScan
 * \ingroup core
 * \brief String scan helper converting string into values in various formats
 */
class StrScan
{
public:
    StrScan(const char* str, size_t length) : str_(str), length_(length) {}
    StrScan(const char* begin, const char* end) : str_(begin), length_(size_t(end-begin)) {}
    explicit StrScan(const std::string& str) : str_(str.c_str()), length_(str.length()) {}
    explicit StrScan(const char* str) : str_(str), length_(strlen(str)) {}

    const char* c_str() const { return str_; }
    size_t length() const { return length_; }
    bool empty() const { return str_==nullptr; }
    bool done() const { return length_==pos_; }
    char current () const { return *(str+pos_); }
    size_t pos () const { return pos_; }

    uint64_t toUnsigned ();
    int64_t toInteger ();
    uint64_t toUnsignedFromDec ();
    uint64_t toUnsignedFromHex ();
    uint64_t toUnsignedFromOct ();
 
    StrScan& operator >> (int8_t& n) { n=toInteger<int8_t>(); return *this; };
    StrScan& operator >> (int16_t& n) { n=toInteger<int16_t>(); return *this; };
    StrScan& operator >> (int32_t& n) { n=toInteger<int32_t>(); return *this; };
    StrScan& operator >> (int64_t& n) { n=toInteger<int64_t>(); return *this; };
    StrScan& operator >> (uint8_t& n) { n=toInteger<uint8_t>(); return *this; };
    StrScan& operator >> (uint16_t& n) { n=toInteger<uint16_t>(); return *this; };
    StrScan& operator >> (uint32_t& n) { n=toInteger<uint32_t>(); return *this; };
    StrScan& operator >> (uint64_t& n) { n=toInteger<uint64_t>(); return *this; };
 
	size_t split (std::vector<std::string>& substrings,
		char separator,
		bool skip_leading_sp,
		bool skip_trailing_sp
	);

private:
    const char* str_ { nullptr };
	size_t length_ { 0 };
	size_t pos_ { 0 };
};

uint64_t StrScan::toUnsigned ()
{
    if (pos_ >= length_)
    {
        return 0;
    }
	char const *p = str_ + pos_;
    if (*p=='0')
    {
        ++p;
        ++pos_;
        if (*p=='x' || *p=='X')
        {
            return toUnsignedFromHex();
        }
        else if (uint64_t(*p - '0') < 8)
        {
            return toUnsignedFromOct();
        }  
    }
    return toUnsignedFromDec();
}

uint64_t StrScan::toUnsignedFromDec ()
{
    uint64_t val = 0;
	uint64_t diff;
    char const *p = str_ + pos_;
    while (pos_< length_ && (diff = uint64_t(*p - '0')) < 10)
    {
      	val = val * 10 + diff;
        ++p;
	    ++pos_;
    }
    return val;
}

uint64_t StrScan::toUnsignedFromHex ()
{
    uint64_t val = 0;
	char const *p = str_ + pos_;
    uint64_t diff;
    while (pos_< length_)
    {
        if ((diff = uint64_t(*p - '0')) < 10)
        {
            val = (val << 4) + diff;
        }
        else if ((diff = uint64_t(*p - 'A')) < 6)
        {
            val = (val << 4 ) + 10 + diff;
        }
        else if ((diff = uint64_t(*p - 'a')) < 6)
        {
            val = (val << 4 ) + 10 + diff;
        }
        else
        {
            break;
        }
        
        ++p;
	    ++pos_;
    }
    return val;
}

uint64_t StrScan::toUnsignedFromOct ()
{
    uint64_t val = 0;
	char const *p = str_ + pos_;
    uint64_t diff;
    while (pos_< length_ && (diff = uint64_t(*p - '0')) < 8)
    {
        val = (val << 3) + diff;
        ++p;
	    ++pos_;
    }
    return val;
}

int64_t StrScan::toInteger ()
{
	char const *p = str_ + pos_;
	if (*p=='-')
    {
        ++pos_;
		return -int64_t(toUnsigned());
    }
	return int64_t(toUnsigned());
}

}
