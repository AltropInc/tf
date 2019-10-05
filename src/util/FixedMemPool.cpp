#include "FixedMemPool.h"
#include <cstdlib>        // for malloc and free
#include <stdexcept>      // for runtime_error
#include <string>         // for string used by runtime_error

namespace tf
{
//-----------------------------------------------------------------------------
// class FixedMemPool
//-----------------------------------------------------------------------------
struct FixedMemPool::EntryHeader
{
    union
    {
        EntryHeader*       next_free_entry_;
        uint64_t           uint64_header_;
        struct
        {
            uint64_t        magic_word_   : 16;  // 0XA3C5
            uint64_t        bin_          : 16;
            uint64_t        ref_count_    : 32;
        }
        header_struct_;
    };

    static constexpr uint16_t MAGIC_WORD = 0XA3C5;
#if (TF_ENDIAN == TF_ENDIAN_BIG)
    static constexpr uint64_t INITIAL_HEADER_VALUE = 0xC5A3000000000001ULL;
#else
    static constexpr uint64_t INITIAL_HEADER_VALUE = 0X10000A3C5ULL;
#endif
    void setAllocated(uint16_t bin)
    {
        uint64_header_ = INITIAL_HEADER_VALUE;
        header_struct_.bin_ = bin;
    }
};

FixedMemPool::FixedMemPool(size_t value_size, size_t entry_num_per_bucket, bool lazy_alloc)
    : value_size_(value_size)
    , entry_size_ (sizeof(EntryHeader)+constAlign(value_size,8))
    , entry_num_per_bucket_(entry_num_per_bucket)
{ 
    if (!lazy_alloc)
    {  
        head_ = newBucket();
    }
}

FixedMemPool::~FixedMemPool()
{
    for (auto bucket: bucket_list_)
    {
        ::free(bucket);
    }
}

void* FixedMemPool::alloc(uint16_t bin)
{
    if (!head_)
    {
        head_ = newBucket();
    }

    EntryHeader* cur_head = head_;
    head_ = cur_head->next_free_entry_;
    cur_head->setAllocated(bin);
    return cur_head+1;
}

void FixedMemPool::free(void* p)
{
    EntryHeader* cur_head = reinterpret_cast<EntryHeader*>(p) - 1;

    if (EntryHeader::MAGIC_WORD != cur_head->header_struct_.magic_word_)
    {
        throw std::runtime_error(std::string("FixedMemPool::free: memory currupted"));
    }

    cur_head->next_free_entry_ = head_;
    head_ = cur_head;
}

FixedMemPool::EntryHeader* FixedMemPool::newBucket()
{
    void* bucket = ::malloc(entry_size_*entry_num_per_bucket_);
    if (!bucket)
    {
        throw std::runtime_error(std::string("FixedMemPool::newBucket: memory full"));
    }
    uint8_t* p = reinterpret_cast<uint8_t*>(bucket);
    EntryHeader* entry=reinterpret_cast<EntryHeader*>(p);
    for ( int i=0; i<entry_num_per_bucket_-1; ++i )
    {
        p+=entry_size_;
        entry->next_free_entry_ = reinterpret_cast<EntryHeader*>(p);
        entry=reinterpret_cast<EntryHeader*>(p);
    }
    entry->next_free_entry_ = nullptr;
    bucket_list_.push_back(bucket);
    return reinterpret_cast<EntryHeader*>(bucket);
}


//-----------------------------------------------------------------------------
// class MemPool
//-----------------------------------------------------------------------------
MemPool& MemPool::instance()
{
    static MemPool s_pool;
    return s_pool;
}

MemPool::~MemPool( )
{
    for (size_t i=0; i<POOL_NUMBER; ++i) delete pools_[i];
}

inline void MemPool::free(size_t bin, void* p)
{
    if (bin >= POOL_NUMBER || nullptr == pools_[bin])
    {
        throw std::runtime_error(std::string("MemPool::fedd: currupted memory"));
    }
    reinterpret_cast<FixedMemPool*>(pools_[bin])->free(p);
}

inline void* MemPool::alloc(size_t bin, size_t entry_size)
{
    if (nullptr == pools_[bin])
    {
        size_t entry_num_per_bucket = (POOL_NUMBER-bin)*100;
        pools_[bin] = new FixedMemPool(entry_size, entry_num_per_bucket);
    }
    return pools_[bin]->alloc(bin);
}

void* MemPool::alloc(size_t size)
{
    int bin = size <= 8 ? 0 : log2Floor(size-1) - 2;
    if (bin >= POOL_NUMBER)
    {
        throw std::runtime_error(std::string("MemPool::alloc: size too big"));;
    }

    return alloc(bin, size);
}

void MemPool::free(void* p)
{
    FixedMemPool::EntryHeader* cur_head =
        reinterpret_cast<FixedMemPool::EntryHeader*>(p) - 1;
    uint16_t bin =  cur_head->header_struct_.bin_;
    free(bin, p);
}

} // name space tf


 
