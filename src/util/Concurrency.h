/**
 * @file Concurrency.h
 * @brief Defines classes used by threads and processes
 * @author David Shang
 * @version 1.00
 * $Id: $
 */

#pragma once
#include <mutex>      // For mutex
#include <atomic>     // For atomic
#include <cassert>    // For assert
#include <cstring>    // For memcpy
#include  <condition_variable>  // For condition_variable


#if defined (__GNUC__)
#include <xmmintrin.h> // For _mm_pause
#endif

namespace tf{

#if defined (__GNUC__)
TF_INLINE void pause () { _mm_pause(); }
TF_INLINE void barrier () { asm volatile("":::"memory"); }
TF_INLINE void fence () {  __sync_synchronize(); }
#else
TF_INLINE void pause () { }
TF_INLINE void barrier () { }
TF_INLINE void fence () { }
#endif

//-----------------------------------------------------------------------------
/**
 * @defgroup WritePolicy Write Protection Policy
 * To use the policy, call aquireWritePermit to get permit:
 * @code auto permit = policy_.aquireWritePermit();
 * @endcode
 * The permit will be automatically release on scope exit
 * @{
*/

struct MutexNone
{
  public:
    void lock() {}
    void unlock() noexcept {}
    bool try_lock() { return true; }
};

class SpinMutex
{
    std::atomic_flag  flag_ {false};
  public:
    SpinMutex() {}
    TF_INLINE void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
            // Not really pause, just tell cpu taht we are in spinning
            pause();
        }
    }
    TF_INLINE bool tryLock()
    {
        return flag_.test_and_set(std::memory_order_acquire);
    }
    TF_INLINE void unlock()
    {
        flag_.clear(std::memory_order_release);
    }
  private:
    SpinMutex(const SpinMutex&) = delete;
    SpinMutex(SpinMutex&&) = delete;
	SpinMutex& operator = (const SpinMutex&) = delete;
	SpinMutex& operator = (SpinMutex&&) = delete;
};

using ScopedNoneLock = std::scoped_lock<MutexNone>;
using ScopedMutexLock = std::scoped_lock<std::mutex>;
using ScopedSpinLock = std::scoped_lock<SpinMutex>;

class WriteSequencer
{
    std::atomic<int64_t>  allocated_ {-1};
    std::atomic<int64_t>  committed_ {-1};
  public:
    TF_INLINE int64_t getAllocated() const { return allocated_; }
    TF_INLINE int64_t getCommitted() const { return committed_; }
    TF_INLINE int64_t acquire(int64_t num =1)
    {
        int64_t seq = allocated_;
        while (!allocated_.compare_exchange_strong(
                seq, seq+num,
                std::memory_order_acq_rel, std::memory_order_relaxed)
              )
        {
            pause();
        }
        return seq + 1;
    }
    template <class Container>
    TF_INLINE void commit(int64_t seq_to_commit, const Container& container)
    {
        int64_t committed = committed_;
        if (seq_to_commit > committed)
        {
            int64_t first_undone = committed + 1;
            for (; first_undone<=seq_to_commit; ++first_undone)
            {
                if (!container[first_undone].isDone())
                {
                    break;
                }
            }
            int64_t last_done = first_undone - 1;
            if (last_done > committed)
            {
                while (!committed_.compare_exchange_strong(
                        committed, last_done,
                        std::memory_order_acq_rel, std::memory_order_relaxed)
                    )
                {
                    if (committed >= last_done)
                    {
                        // someone else already updated the committed
                        break;
                    }
                    pause();
                }
            }
        }
    }
};

template <typename Mutex, typename CV>
class SemaphoreT 
{
public:
    using native_handle_type = typename CV::native_handle_type;
    explicit SemaphoreT(size_t init_count=0): count_ {init_count} {}

    TF_INLINE void notify()
    {
        std::lock_guard<Mutex> lock {mutex_};
        ++count_;
        cv_.notify_one();
    }

    TF_INLINE void wait()
    {
        std::unique_lock<Mutex> lock{mutex_};
        cv_.wait(lock, [&]{ return count_ > 0; });
        --count_;        
    }

    TF_INLINE bool try_wait()
    {
        std::lock_guard<Mutex> lock{mutex_};
        if (count_ > 0)
        {
            --count_;
            return true;
        }
        return false;        
    }

    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& d)
    {
        std::unique_lock<Mutex> lock{mutex_};
        auto finished = cv_.wait_for(lock, d, [&]{ return count_ > 0; });
        if (finished)
            --count_;
        return finished;
    }

    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& t)
    {
        std::unique_lock<Mutex> lock{mutex_};
        auto finished = cv_.wait_until(lock, t, [&]{ return count_ > 0; });
        if (finished)
            --count_;
        return count_;
    }

    native_handle_type native_handle()
    {
        return cv_.native_handle();
    }

private:
    SemaphoreT(const SemaphoreT&) = delete;
    SemaphoreT(SemaphoreT&&) = delete;
    SemaphoreT& operator=(const SemaphoreT&) = delete;
    SemaphoreT& operator=(SemaphoreT&&) = delete;
    Mutex    mutex_;
    CV       cv_;
    size_t   count_;
};

using Semaphore = SemaphoreT<std::mutex, std::condition_variable>;

class SpinSemaphore
{
public:
    SpinSemaphore (uint64_t init_count=0): count_ {init_count} {}
    TF_INLINE void notify() {
        count_.fetch_add(1, std::memory_order_release);
    }

    TF_INLINE bool try_wait() {
        uint64_t count = count_;
        if (count > 0)
        {
            return count_.compare_exchange_strong(
                        count, count-1,
                        std::memory_order_acq_rel, std::memory_order_relaxed);
        }
        return false;
    }

    TF_INLINE void wait()
    {
        while(!try_wait())
        {
            pause();
        }
    }

private:
    std::atomic<uint64_t> count_;
};

template <typename SemaphoreT>
class ScopedSemaphore
{
public:
    explicit ScopedSemaphore(SemaphoreT& sem) : semaphore_(sem)
    { semaphore_.wait(); }
    ~ScopedSemaphore() { semaphore_.notify(); }
private:
    ScopedSemaphore(const ScopedSemaphore&) = delete;
    ScopedSemaphore(ScopedSemaphore&&) = delete;
    ScopedSemaphore& operator=(const ScopedSemaphore&) = delete;
    ScopedSemaphore& operator=(ScopedSemaphore&&) = delete;
    SemaphoreT& semaphore_;
};

}