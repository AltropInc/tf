#pragma once
#include "Platform.h"    // For WritePolicy
#include "Concurrency.h"    // For WritePolicy
#include <stddef.h>
#include <vector>

#ifdef TEST_BUILD
#include "StrScan.h"        // For split
#include <iostream>         // For std::cout
#endif

namespace tf{

/**
 * \class StrPoolBase
 * \ingroup StringUtils
 * \brief A string buffer to store commonly used string constants.
 * This is a base class of StrPool_T. When string is pooled, no release
 * from the pool.
 */
class StrPoolBase
{
  protected:
    struct Page
    {
        char* buffer_ {nullptr};
        size_t pos_   {0};
        Page (size_t sz)
        {
            buffer_ = new char[sz];
        }
        ~Page ()
        {
            delete [] buffer_;
        }
        Page (Page && other) noexcept
          : buffer_ (other.buffer_), pos_(other.pos_)
        {
            // Move constructor to transfer resources. After transfer,
            // invalidate resources in other. This Move constructor is
            // required to prevent std::vector to use default copy
            // constructor where the distructor will be called to
            // delete the buffer_ when page vector resizing.
            other.buffer_ = nullptr;
            other.pos_ = 0;
        }
        Page (const Page & other) = delete;
    };

    StrPoolBase (size_t page_size)
      : page_size_(page_size)
    {
        newPage();
    }

    void newPage();

    const char* push(const char* str, size_t length);

    const char* push(
        const std::initializer_list<const char*>& strs,
        const std::initializer_list<size_t>& sizes,
        size_t total_length,
        const char* separator);

#ifdef TEST_BUILD
  public:
    void dump();
#endif

    std::vector<Page> pages_;
    size_t page_size_;
    Page* cur_page_ {nullptr};
};

/**
 * \class StrPool_T
 * \ingroup StringUtils
 * \brief A string buffer to store commonly used string constants.
 *        No release of strings in the pool.
 */
template<class  MutexT = MutexNone>
class StrPool_T: public StrPoolBase
{
  public:
    StrPool_T (size_t page_size=1024)
      : StrPoolBase(page_size)
    {}

    const char* push(const char* str, size_t length)
    {
        const char* ret (nullptr);
        assert(length < page_size_);
        {
            std::scoped_lock scope_lock {mutex_};
            ret = StrPoolBase::push(str, length);
        }
        return ret; 
    }

    const char* push(const char* str)
    {
        return push(str, std::strlen(str));
    }

    const char* push(const std::string& str)
    {
        return push(str.c_str(), str.length());
    }

    const char* push(
        const std::initializer_list<const char*>& strs,
        const std::initializer_list<size_t>& str_sizes,
        size_t total_length,
        const char* separator)
    {
        const char* ret (nullptr);
        {
            std::scoped_lock scope_lock {mutex_};
            ret = StrPoolBase::push(strs, str_sizes, total_length, separator);
        }
        return ret;
    }

    const char* push(std::initializer_list<const char*> strs, char separator)
    {
        size_t str_sizes[strs.size()];
        size_t total_length (0);
        size_t i (0);
        for (const char* str : strs)
        {
            str_sizes[i] = strlen(str);
            total_length += str_sizes[i++];
        }
        total_length += strs.size()-1;    // add number of separators
        return push(strs, str_sizes, total_length, separator);
    }
  private:
    MutexT    mutex_;
};

using StrPool = StrPool_T<MutexNone>;
using StrPoolMutexLocked = StrPool_T<std::mutex>;
using StrPoolSpinLocked = StrPool_T<SpinMutex>;

}


