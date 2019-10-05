#pragma once

#include "Platform.h"
#include "Intrinsics.h"

#include <vector>

namespace tf
{

class MemPool;

class FixedMemPool
{
  public:
      
    FixedMemPool(size_t value_size,
            size_t entry_num_per_bucket=100,
            bool lazy_alloc = true);

    ~FixedMemPool();

    void* alloc(uint16_t bin = 0) noexcept(false);

    void free(void* p) noexcept(false);

  private:
    friend class MemPool;

    struct EntryHeader;
    EntryHeader* newBucket()  noexcept(false);
    EntryHeader*              head_;
    size_t                    value_size_;
    size_t                    entry_size_;
    size_t                    entry_num_per_bucket_;
    std::vector<void*>        bucket_list_;
};

//-----------------------------------------------------------------------------
template <typename  T>
class FixedPool: public FixedMemPool
{
  public:
    FixedPool(size_t entry_num_per_bucket, bool lazy_alloc = true)
        : FixedMemPool(sizeof(T), entry_num_per_bucket, lazy_alloc) {}

    T* alloc() noexcept(false)
    {
        void* p = FixedMemPool::alloc();
        return new (p) T;
    }

    void free(T* v) noexcept(false)
    {
        v->~T();
        FixedMemPool::free(v);
    }
};

//-----------------------------------------------------------------------------
class MemPool
{
  public:

    static MemPool& instance();

    constexpr static size_t MAX_VALUE_SIZE = 8192;
    constexpr static size_t POOL_NUMBER = constLog2(MAX_VALUE_SIZE)-2;

    template <typename T, typename... Args>
    T* acq(Args... args) noexcept(false)
    {
        constexpr size_t sz_aligned = constAlign(sizeof(T),8);
        static_assert (sz_aligned <= MAX_VALUE_SIZE);
        constexpr uint32_t pool_index = sz_aligned <= 8 ? 0 : constLog2(sz_aligned-1) - 2;
        void *p = alloc(pool_index, sz_aligned);
        return p ? new(p)T(args...) : (T*)nullptr;
    }

    template <typename T>
    void del(T* p) noexcept(false)
    {
        if (p)
        {
            constexpr size_t sz_aligned = constAlign(sizeof(T),8);
            constexpr uint32_t pool_index = sz_aligned <= 8 ? 0 : constLog2(sz_aligned-1) - 2;
            p->~T();
            free(pool_index, (void*)p);
        }
    }

    void free(void* p) noexcept(false);
    void* alloc(size_t entry_size) noexcept(false);

  private:

    ~MemPool();

    void free(size_t bin, void* p) noexcept(false);
    void* alloc(size_t bin, size_t entry_size) noexcept(false);

    FixedMemPool* pools_ [POOL_NUMBER] {nullptr};
};

} // name space tf


 
