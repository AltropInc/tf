#pragma once

#include "Platform.h"
#include "Enum.h"
#include <string>
#include <errno.h>

namespace tf {

class SharedMemoryImpl;
class File;

ENUM (SM_Mode, uint8_t, SM_CreateOnly, SM_OpenOrCreate, SM_OpenOnly);
ENUM (SM_Access, uint8_t, SM_ReadOnly, SM_ReadWrite);
ENUM (SM_LifeSpan, uint8_t, SM_LifeProcess, SM_LifeSystem);
ENUM (SM_ShareMode, uint8_t, SM_ShareIPC, SM_ShareLocal);

/**
 * \class SharedMemory
 * \ingroup IPC
 * \brief Shared Memory used for inter process communication
 * SM_Mode: SM_CreateOnly - create a shared memory with a given name
 *               it will fail if the one with the same name exists
 *          SM_OpenOrCreate - open a shared memory with a given name
 *               if it does not exist, create a new
 *          SM_OpenOnly - open a shared memory with a given name,
 *               fail if it does not exist
 * Only the owner can use SM_CreateOnly or SM_OpenOrCreate mode.
 * 
 */
class TF_CORE_PUBLIC SharedMemory
{
public:

    struct SM_Header
    {
        // A flag to indicate the status of the shared memory
        // Only the master can update the status. 
        std::atomic<uint64_t>   flags_; 
    };

	SharedMemory(const std::string& name, bool is_master);
    
    /// No copy and moving constructor
	SharedMemory(SharedMemory&& moved) = delete;
    SharedMemory(SharedMemory& moved) = delete;
    SharedMemory& operator=(SharedMemory&& moved) = delete;

    SM_LifeSpan leftSpan() const { return SM_LifeSystem; }
    SM_ShareMode shareMode() const { return SM_ShareIPC; }

	~SharedMemory();

	int acqire (const std::string& name,
               SM_Mode mode,
               SM_Access permision,
               std::size_t size);
    void release ();

    /// Set the ready falg by the owner to indicate the memory is ready
    /// for clients to read
    void setReady(bool ready);

    const char *getName() const;
    int getHandle() const;

	void swap(SharedMemory& other);

	char* ptr() const;
	const char* ptr();
	size_t size() const;
	SM_Header* getHeader() const;

private:

    std::string    name_;
    std::string    shm_name_;
    const bool     persistent_;
    int            handle_;
    bool           is_owner_;
    size_t         total_size_;
    size_t         header_size_;
    size_t         payload_size_;
    void*          address_;
    SM_Header*     header_;
    char*          payload_;
    bool           is_new_;
};

class TF_CORE_PUBLIC LocalMemory
{
public:
	LocalMemory(const std::string& name, bool is_master);
    
    /// No copy and moving constructor
	LocalMemory(SharedMemory&& moved) = delete;
    LocalMemory(SharedMemory& moved) = delete;
    LocalMemory& operator=(SharedMemory&& moved) = delete;

    SM_LifeSpan leftSpan() const { return SM_LifeProcess; }
    SM_ShareMode shareMode() const { return SM_ShareLocal; }

	~LocalMemory();

	int acqire (const std::string& name,
               SM_Mode mode,
               SM_Access permision,
               std::size_t size);
    void release ();

    const char *getName() const;

	char* ptr() const;
	const char* ptr();
	char* HeaderPtr() const;
	const char* HeaderPtr();
	size_t size() const;

private:
    std::string    name_;
    std::string    shm_name_;
    int            handle_;
    bool           is_master_;
    size_t         total_size_;
    size_t         header_size_;
    size_t         payload_size_;
    void*          address_;
    void*          header_;
    char*          payload_;
    bool           is_new_;
};

template <class StorageT, class ContainedT>
class TF_CORE_PUBLIC SharedContainer
{
    StorageT            storage_;
    ContainedT&         contained_;
  public:
	SharedContainer(
        const std::string& name,
        bool is_master,
        ContainedT& contained
    ) : storage_ (name, is_master)
      , contained_ (contained)
    {
    }

    int init()
    {
        size_t size = contained_.requiredSize();
        size_t header_size = contained_.requiredHeaderSize();
        SM_Mode mode = contained_.getOpenMode(is_master);
        SM_Access access = contained_.getAccessRequeust(is_master);
        if (storage_.alloc(mode, access, size, header_size) < 0)
        {
            return -1;
        }
        contained.init(storage_.ptr(), storage_.isNew(), is_master);
        return 0;
    }
 };

// A lock free shared memory queue with multiple writers and readers
class TF_CORE_PUBLIC SharedQueue
{
    struct QueueHeader
    {
        WriteSequencer     write_sn_;
        size_t             entry_size_;
        size_t             entry_number_;
    };

    QueueHeader *   queue_header_ {nullptr};
    char *          entry_buffer_ {nullptr};
    size_t          entry_number_;
    size_t          entry_size_shift_;
    size_t          entry_size_;
    size_t          entry_size_mask_;
    size_t          overrun_cnt_ {0};
  public:
    struct EntryHeader
    {
        uint64_t     sequence_;
    };
    SharedQueue (
        size_t entry_size,
        size_t entry_number
    );

    size_t requiredSize();
    void init(char* addr, bool is_new);

    TF_INLINE EntryHeader& operator[] (int64_t seq)
    {
        return reinterpret_cast<EntryHeader&>(
            *(entry_buffer_ + ((seq & entry_size_mask_) << entry_size_shift_)));
    }
    TF_INLINE const EntryHeader& operator[] (int64_t seq) const
    {
        return reinterpret_cast<EntryHeader&>(
            *(entry_buffer_ + ((seq & entry_size_mask_) << entry_size_shift_)));
    }

    TF_INLINE EntryHeader* getNextWriteEntry()
    { 
        int64_t seq = queue_header_->write_sn_.acquire();
        EntryHeader& header = (*this)[seq];
        header_.sequence_ = seq;
        return &header_;
    }

    TF_INLINE EntryHeader* getNextNWriteEntry(int num =1)
    {
        int64_t seq = queue_header_->write_sn_.acquire(num);
        for (int64_t s=seq; s<=seq+num; ++s)
        {
            EntryHeader& header = (*this)[s];
            header_.sequence_ = s;
        }
        return  (*this)[seq];
    }

    TF_INLINE void commitWriteSeq(int64_t seq)
    { return queue_header_->write_sn_.commit(seq, *this); }

    TF_INLINE bool empty() const
    { return queue_header_->write_sn_.getCommitted()<0; }
 
    TF_INLINE int64_t getEarliestCommit() const
    {
        int64_t comitted = queue_header_->write_sn_.getCommitted();
        int64_t allocated = queue_header_->write_sn_.getAllocated();
        int64_t uncommitted = allocated - comitted;
        if (allocated - entry_number_ >0)
        {
            comitted -= entry_number_ - uncommitted;
        }
        return comitted;
    }

    TF_INLINE const EntryHeader* getReadEntry(int64_t seq) const
    {
        int64_t allocated = queue_header_->write_sn_.getAllocated();
        if (seq <= allocated - entry_number_)
        {
            overrun_cnt_ += allocated - entry_number_ - seq + 1;
            return nullptr;
        }
        return *this[seq];
    }

    TF_INLINE const bool isReadEntryValid(int64_t seq) const
    {
        if (TF_LIKELY(*this[seq].sequence_ == seq))
        {
            return true;
        }
        ++overrun_cnt_;
        return false;
    }
};

SharedQueue::SharedQueue(
    size_t entry_size,
    size_t entry_number
) : entry_number_(entry_number)
  , entry_size_shift_ (log2Ceil(constAlign(entry_size+sizeof(EntryHeader),
                                         CPUInfo::cache_alignment_)))
  , entry_size_(1<<entry_size_shift_)
  , entry_size_mask_ (entry_size_-1)
{
}

size_t SharedQueue::requiredSize()
{
    return constAlign(sizeof(QueueHeader),CPUInfo::cache_alignment_) +
           (entry_number_ << entry_size_shift_);
}

void SharedQueue::init(char* addr, bool is_new)
{
    if (is_new)
    {
        queue_header_ = new (addr) QueueHeader();
    }
    else
    {
        queue_header_ = reinterpret_cast<QueueHeader*>(addr);
    }
    
    entry_buffer_ = addr + constAlign(sizeof(QueueHeader),CPUInfo::cache_alignment_);
}



} // namespace tf

