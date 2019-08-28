#pragma once
#include "types.h"
#include <stddef.h>
#include <string>
#include <tuple>

namespace tf {

class StrBuf
{
  public:
    StrBuf (char* buffer, size_t sz, size_t filled=0)
      : buffer_(buffer)
	  , capacity_(sz)
	  , tail_(filled)
	{}

    StrBuf (std::string & buffer, size_t filled=0)
      : buffer_(&buffer[0])
	  , capacity_(buffer.length())
	  , tail_(filled)
	{}

    size_t length() const { return tail_; }
    bool empty() const { return tail_==0; }
    bool overflowed() const { return tail_==capacity_; }

    const char* c_str() const
    {  
       if (tail_ < capacity_) *const_cast<char*>(buffer_+tail_) ='\0';
       return buffer_;
    }

    void clear() { tail_ = 0; }
	
    void resize(size_t sz, char fill=0)
    {
        if (tail_ >= sz) tail_ = sz;
        else if (sz < capacity_)
        {
            while (tail_ < sz) buffer_[tail_++] = fill;
        }
    }

    void push_back (char val) { if (tail_ < capacity_) buffer_[tail_++] = val; }
    void pop_back () { if (tail_>0) --tail_; }
    const char& back () const { return tail_>0 ? buffer_[tail_-1] : buffer_[0]; }
    char& back () { return tail_>0 ? buffer_[tail_-1] : buffer_[0]; }

    void append (const char* val)
    { while (tail_ < capacity_ && *val) buffer_[tail_++] = *val++; }

    void append (const char* val, size_t n)
    { while (tail_ < capacity_ &&  *val && n--) buffer_[tail_++] = *val++; }

    void append (const std::string val)
    { append(val.c_str(), val.length()); }

    void append (size_t repeat, char val)
    { while (tail_ < capacity_ && repeat--) buffer_[tail_++] = val; }

    void appendUntil (const char* val, char terminator)
    { while (tail_ < capacity_ && *val!=terminator) buffer_[tail_++] = *val++; }

    void appendWithPadding (const char* val, size_t n, char padding)
    {
       while (tail_ < capacity_ &&  *val && n--) buffer_[tail_++] = *val++;
       while (tail_ < capacity_ && n--) buffer_[tail_++] = padding;
    }
  private:
    char*  buffer_;
    size_t capacity_;
    size_t tail_;
};

template <typename BufferT>
class StrPrint
{
  public:
    explicit SPrint(BufferT& buffer) : buffer_(buffer) {}

    const char* c_str() const { return buffer_.c_str(); }

    size_t length() const { return buffer_.length(); }
    bool empty() const { return buffer_.empty(); }
    bool overflowed() const
    { return std::is_same<std::string, BufferT>::value ? false : buffer_.overflowed(); }

    void clear() { buffer_.clear(); }
   
    // Append char, and string
    StrPrint& operator << (char val) { buffer_.push_back(val); return *this; }
    StrPrint& operator << (const char * val) { buffer_.append(val); return *this; }
    StrPrint& operator << (const std::string& val) { buffer_.append(val); return *this; }

    // Use tuple because operator << can only take a single parameter
    StrPrint& operator << (std::tuple<const char *, size_t> val)
    { buffer_.append(std::get<0>(val), std::get<1>(val)); return *this;  }
    StrPrint& operator << (std::tuple<size_t, char> val)
    { buffer_.append(std::get<0>(val), std::get<1>(val)); return *this; }
    
    StrPrint& operator << (uint32_t);
    StrPrint& operator << (int32_t);
    StrPrint& operator << (uint64_t);
    StrPrint& operator << (int64_t);
    StrPrint& operator << (uint16_t val) { return *this << uint32_t(val); }
    StrPrint& operator << (int16_t val) { return *this << int32_t(val); }
    StrPrint& operator << (uint8_t val) { return *this << uint32_t(val); }
    StrPrint& operator << (int8_t val) { return *this << int32_t(val); }

    StrPrint& operator << (double val);
    StrPrint& operator << (std::tuple<double, int> val);
   
  private:
    BufferT&    buffer_;
};

namespace ConstDef
{
	extern const char s_double_digits[];
	extern const uint64_t s_exp10[];
};

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (uint32_t val)
{
    if (val < 10)
    {
        buffer_.push_back('0' + val);
    }
    else if (val < 100)
    {
        buffer_.append(ConstDef::s_double_digits + val*2, 2);
    }
    else if (val < 10000)
    {
        unsigned high = val / 100;
        unsigned low = val % 100;
        *this << high;
        if (low <10)
        {
            buffer_.push_back('0' + low);
        }
        *this << low;
    }
    else
    {
        unsigned high = val / 10000;
        unsigned low = val % 10000;
        *this << high;
        if (low <1000) buffer_.push_back('0');
        if (low <100) buffer_.push_back('0');
        if (low <10) buffer_.push_back('0');
        *this << low;
    }
    return *this;
}

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (int32_t val)
{
    if (val >= 0)
    {
       return *this << uint32_t(val);
    }
    buffer_.push_back('-');
    return *this << uint32_t(-val);
}

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (uint64_t val)
{
    if (val < 10000000)
    {
        return *this << (uint32_t)val;
    }
    uint64_t high = val / 10000000;
    uint64_t low = val % 10000000;
    *this << high;
    if (low <1000000) buffer_.push_back('0');
    if (low <100000) buffer_.push_back('0');
    if (low <10000) buffer_.push_back('0');
    if (low <1000) buffer_.push_back('0');
    if (low <100) buffer_.push_back('0');
    if (low <10) buffer_.push_back('0');
    return *this << low;
}

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (int64_t val)
{
    if (val >= 0)
    {
       return *this << uint64_t(val);
    }
    buffer_.push_back('-');
    return *this << uint64_t(-val);
}

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (double val)
{
    return *this << std::make_tuple(val,6);
}

template<typename BufferT>
TF_INLINE StrPrint<BufferT>& StrPrint<BufferT>::operator << (std::tuple<double, int> val)
{
    size_t precision = std::get<1>(val);
    double dval = std::get<0>(val);
    if (dval < 0)
    {
        buffer_.push_back('-');
        dval = -dval;
    }
    uint64_t exp = ConstDef::s_exp10[precision];
    int64_t lval = int64_t(dval * exp + 0.5);
    *this << int64_t(lval / exp) << '.';
    int64_t fval = lval % exp;
    for (int p = precision - 1; p > 0 && fval < ConstDef::s_exp10[p]; --p)
    {
       buffer_.push_back('0');
    }
    return *this << fval;
}

}
