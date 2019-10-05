#pragma once
#include "Intrinsics.h"   // for strHash

#include <string>
#include <cstring>
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
	  { }

    size_t length() const { return tail_; }
    bool empty() const { return tail_==0; }
    bool overflowed() const { return tail_==capacity_; }

    const char* c_str() const
    {  
       if (tail_ < capacity_) *const_cast<char*>(buffer_+tail_) ='\0';
       return buffer_;
    }

    void terminate() const
    {  
       if (tail_ < capacity_) *const_cast<char*>(buffer_+tail_) ='\0';
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

    StrBuf& append (const char* val)
    {
        if (val) while (tail_ < capacity_ && *val) buffer_[tail_++] = *val++;
        return *this;
    }

    StrBuf& append (const char* val, size_t n)
    {
        if (val) while (tail_ < capacity_ &&  *val && n--) buffer_[tail_++] = *val++;
        return *this;
    }

    StrBuf& append (const std::string val)
    { append(val.c_str(), val.length()); }

    StrBuf& append (size_t repeat, char val)
    {
        while (tail_ < capacity_ && repeat--) buffer_[tail_++] = val;
        return *this;
    }

    StrBuf& appendUntil (const char* val, char terminator)
    {
        if (val) while (tail_ < capacity_ && *val!=terminator) buffer_[tail_++] = *val++;
        return *this;
    }

    StrBuf& appendPostPadding (const char* val, size_t n, char padding)
    {
        if (val) while (tail_ < capacity_ &&  *val && n) { buffer_[tail_++] = *val++; --n; }
        while (tail_ < capacity_ && n--) buffer_[tail_++] = padding;
        return *this;
    }

    StrBuf& appendPrePadding (const char* val, size_t val_len, size_t n, char padding)
    {
        size_t free_space = capacity_ - tail_;
        while (free_space-- > val_len && n > val_len) { buffer_[tail_++] = padding; --n; }
        if (val) while (tail_ < capacity_ &&  *val && n--) buffer_[tail_++] = *val++;
        return *this;
    }

  private:
    char*  buffer_;
    size_t capacity_;
    size_t tail_;
};

template <size_t N>
class StrFixed
{
  public:
    StrFixed() { buffer_[0]='\0'; buffer_[N]='\0'; };

    StrFixed (const char* str)
	  {
        StrBuf (buffer_, N).append(str).terminate();
    }

    StrFixed (const char* str, size_t length)
	  {
        StrBuf (buffer_, N).append(str, length).terminate();
    }

    StrFixed (const std::string& str)
	  {
        StrBuf (buffer_, N).append(str.c_str(), str.length()).terminate();
    }

    template<size_t M>
    StrFixed (const StrFixed<M>& str)
	  {
        StrBuf (buffer_, N).append(str.c_str(), M).terminate(); 
    }

    const char* c_str() const
    {
        return buffer_;
    }

    size_t length() const
    {
        return std::strlen(buffer_);
    }

    constexpr size_t capacity() const
    {
        return N;
    }

  private:
    char buffer_[N+1];
};

class StrRef
{
  public:
    StrRef (const char* buffer)
      : buffer_(buffer)
	{}

    size_t length() { return buffer_ ? std::strlen(buffer_) : 0; }
    const char* c_str() { return buffer_; }

    bool operator==(const StrRef &other) const
    { return std::strcmp(buffer_, other.buffer_)==0; }

    size_t hash() const
    {
        return strHash(buffer_);
    }

  private:
    const char*  buffer_ { nullptr }; 
};

class StrRefInLength
{
  public:
    StrRefInLength (const char* buffer)
      : buffer_(buffer)
      , length_(buffer ? std::strlen(buffer) : 0)
	  {}

    StrRefInLength (const char* buffer, size_t length)
      : buffer_(buffer)
      , length_(length)
	  {}

    StrRefInLength (const std::string & str)
      : buffer_(str.c_str())
      , length_(str.length())
	  {}

    size_t length() { return length_; }
    const char* c_str() { return buffer_; }

    bool operator==(const StrRefInLength &other) const
    { return length_==other.length_ && std::strncmp(buffer_, other.buffer_, length_)==0; }
 
    size_t hash() const
    {
        return strHash(buffer_, length_);
    }

  private:
    const char*  buffer_ { nullptr };
    size_t       length_ { 0 };
};

} // namespace tf

/**
 * \class PooledStrKey
 * \ingroup StringUtils
 * \brief A hash key using pooled string.
 */

namespace std {
template <>
struct hash<tf::StrRef>
{
    size_t operator()(const tf::StrRef& key) const noexcept
    {
        return key.hash();
    }
};

template <>
struct hash<tf::StrRefInLength>
{
    size_t operator()(const tf::StrRefInLength& key) const noexcept
    {
        return key.hash();
    }
};
} // namespace std
