#pragma once
#include "Platform.h"
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
template <typename T, class Compare=std::less<T> >
class SortedArray
{
  protected:

    std::vector<T>   array_;
    size_t head_;
    size_t tail_;

    size_t lowBound(const T& x)
    {
        size_t start = head_;
        size_t end = tail_;
        size_t mid;
        while (start < end)
        {
            mid = (start+end)/2;
            //int res = Compare(x, array_[mid]);
            if (Compare()(x, array_[mid]))
            {
                end = mid;
            }
            /* else if (res==0)
            {
                return mid;
            }*/
            else
            {
                start = mid+1;
            } 
        }
        return end;
    }

  public:
    SortedArray(size_t capacity)
        : array_(capacity)
        , head_ {capacity/2}
        , tail_ {capacity/2}
    {}

    void insert (const T& x)
    {
        if (tail_ - head_ >= array_.size())
        {
            size_t new_size = array_.size()*2;
            std::cout<<"RESIZE TO "<< new_size <<std::endl;
            array_.resize(new_size);
            print();
        }
        size_t ix = lowBound(x);
        std::cout << "LB=" << ix << " T=" << tail_ << " H=" << head_;
        if (ix==tail_ && tail_ < array_.size())
        {
            std::cout << " A" << std::endl;
            array_[tail_++] = x;
        }
        else if (ix==head_ && head_>0)
        {
            std::cout << " B" << std::endl;
            array_[--head_] = x;
        }
        else if (ix - head_ < tail_ - ix && head_ > 0 || tail_>=array_.size())
        {
            std::cout << " C" << std::endl;
            std::memmove(&array_[head_-1], &array_[head_], sizeof(T)*(ix-head_));
            --head_;
            array_[ix-1] = x;
        }
        else
        {
            std::cout << " D" << std::endl;;
            std::memmove(&array_[ix+1], &array_[ix], sizeof(T)*(tail_-ix));
            ++tail_;
            array_[ix] = x;
        } 
        print();   
    }

    void print()
    {
        for (size_t i=head_; i<tail_; ++i)
            std::cout << '[' << i << "]: " << array_[i] << std::endl;
    }
};

template <typename Key, typename T, class Compare>
class SortedBukets
{
  public:
    using value_type = std::pair<Key, T>;
    using reference = value_type&;
    using pointer = value_type*;
  protected:

    std::vector<value_type>   buckets_;
    size_t head_;
    size_t tail_;

    size_t lowBound(const Key& x)
    {
        size_t start = head_;
        size_t end = tail_;
        size_t mid;
        while (start < end)
        {
            mid = (start+end)/2;
            int res = Compare()(x, buckets_[mid].first);
            if (res < 0)
            {
                end = mid;
            }
            else if (res==0)
            {
                return mid;
            }
            else
            {
                start = mid+1;
            } 
        }
        return end;
    }

    void erase (size_t ix)
    {
        if (ix - head_ < tail_ - ix)
        {
            std::memmove(&buckets_[head_+1], &buckets_[head_], sizeof(value_type)*(ix-head_));
            ++head_;
        }
        else
        {
            std::memmove(&buckets_[ix-1], &buckets_[ix], sizeof(value_type)*(tail_-ix));
            --tail_;
        } 
    }

  public:
    SortedBukets(size_t capacity)
        : buckets_(capacity)
        , head_ {capacity/2}
        , tail_ {capacity/2}
    {}

    class iterator
    {
        SortedBukets& parent_;
        size_t        ix_;
      public:
        iterator(const iterator& oth): parent_(oth.parent_), ix_(oth.ix_) {}
        iterator(SortedBukets & p, size_t ix): parent_(p), ix_(ix) {}
        bool operator==(const iterator& oth) { return ix_==oth.ix_; }
        iterator& operator=(const iterator&oth) { ix_=oth.ix_; return *this; }
        iterator& operator++() { if (ix_<parent_.tail_) ++ix_; return *this; }
        iterator& operator--() { if (ix_>parent_.head_) --ix_; else ix_=parent_.tail_; return *this; }
        reference operator*() const { return parent_.buckets_[ix_]; }
        pointer operator->() const { return &(parent_.buckets_[ix_]); }
        size_t index() const { return ix_; }
        friend void swap(iterator& lhs, iterator& rhs) { std::swap(lhs.ix_, rhs.ix_); }
    };

    iterator begin() { return iterator(*this, head_); }
    iterator end() { return iterator(*this, tail_); }
    iterator last() { return iterator(*this, tail_==head_?tail_:tail_-1); }

    iterator find(const Key& key)
    {
        size_t ix = lowBound(key);
        return (ix < tail_ && buckets_[ix].first == key)
                ? iterator(*this, ix) : iterator(*this, tail_);
    }

    void pop_front () { if (head_ < tail_) ++head_; }
    value_type& front () { return buckets_[head_]; }
    const value_type& front () const { return buckets_[head_]; }
    void push_front (const Key& key, const T& val)
    {
        if (head_ == 0)
        {
            if (tail_ == buckets_.size())
            {
                buckets_.resize(buckets_.size()*2);
            }
            size_t dist = (buckets_.size() - tail_ + 1) / 2;
            std::memmove(&buckets_[head_+dist], &buckets_[head_], sizeof(value_type)*(tail_-head_));
            head_ += dist;
            tail_ += dist;
        }
        buckets_[--head_] = std::make_pair(key, val);
    }
 
    void pop_back () { if (head_ < tail_) --tail_; }
    value_type& back () { return buckets_[tail_-1]; }
    const value_type& back () const { return buckets_[tail_-1]; }

    size_t size() const { return tail_ - head_; }
    bool empty() const { return tail_ == head_; }
    bool head() const { return head_; }
    bool tail() const { return tail_; }

    void update (const Key& key, const T& val)
    {
        size_t ix = lowBound(key);
        if (ix < tail_ && buckets_[ix].first == key)
        {
            // update the given part in val from existing
            buckets_[ix].second.update(val);
            if (buckets_[ix].second==T(0))
            {
                erase(ix);
            }
        }
    }

    void add (const Key& key, const T& val)
    {
        if (tail_ - head_ >= buckets_.size())
        {
            buckets_.resize(buckets_.size()*2);
        }
        size_t ix = lowBound(key);
        if (ix < tail_ && buckets_[ix].first == key)
        {
            // add the given part in val to existing
            buckets_[ix].second.add(val);
            return;
        }
        if (ix==tail_ && tail_ < buckets_.size())
        {
            buckets_[tail_++] = std::make_pair(key, val);
        }
        else if (ix==head_ && head_>0)
        {
            buckets_[--head_] = std::make_pair(key, val);;
        }
        else if (ix - head_ < tail_ - ix && head_ > 0 || tail_>=buckets_.size())
        {
            std::memmove(&buckets_[head_-1], &buckets_[head_], sizeof(value_type)*(ix-head_));
            --head_;
            buckets_[ix-1] = std::make_pair(key, val);;
        }
        else
        {
            std::memmove(&buckets_[ix+1], &buckets_[ix], sizeof(value_type)*(tail_-ix));
            ++tail_;
            buckets_[ix] = std::make_pair(key, val);
        } 
        print();   
    }

    void print()
    {
        for (size_t i=head_; i<tail_; ++i)
            std::cout << '[' << i << "]: (" << buckets_[i].first
            << ',' <<  buckets_[i].second << ')' << std::endl;
    }
};

template <typename Key, typename T, class Compare>
class SidedBuckets
{
    SortedBukets<Key, T, Compare>   back_bucks_;
    std::vector<T>                  front_bucks_;
    int                             top_ix_ {0};
    int                             bot_ix_ {0};
    Key                             top_ {Compare::max()};
    size_t                          count_ {0};

  public:

    using value_type = std::pair<Key, T>;

    class iterator
    {
        SidedBuckets& parent_;
        size_t        ix_;
        bool          in_front_ { true };
      public:
        iterator(const iterator& oth):
            parent_(oth.parent_),
            ix_(oth.ix_)
            in_front_(oth.in_front_)
            {}

        iterator(SidedBuckets & p, size_t ix, bool in_front):
            parent_(p), ix_(ix), in_front_(in_front) {}

        bool operator==(const iterator& oth)
        {
            return parent_==oth.parent_ && ix_==oth.ix_ &&
                   in_front_==oth.in_front_;
        }

        iterator& operator=(const iterator&oth)
        {
            parent_=oth.parent_;
            ix_=oth.ix_;
            in_front_==oth.in_front_;
            return *this;
        }

        iterator& operator++()
        {
            if (in_front_)
            {
                if (ix_ < parent_.front_bucks_.bot_ix_)
                {
                    ++ix_;
                    while (parent_.isEmptyEntry(ix))
                    {
                        ++ix_;
                        if (ix_ >= parent_.front_bucks_.bot_ix_)
                        {
                            break;
                        }
                    }
                }
                if (ix_ == parent_.front_bucks_.bot_ix_)
                {
                    if (!back_bucks_.empty())
                    {
                        ix_= parent_.back_bucks_.head();
                        in_front_ = false;
                    }
                }
                return *this;
            }
            if (ix_<parent_.back_bucks_.tail()) ++ix_;
            return *this;
        }

        value_type operator*() const
        {
            return in_front_ ? parent_.getFrontEntry(ix_)
                             : parent_.getBackEntry(ix_);
        }
    };

    bool frontEmpty() const { return top_ix_==bot_ix_; }
    bool empty() const { return frontEmpty() && back_bucks_.empty(); }
    size_t size() const { return count_ + back_bucks_.size(); }
    iterator begin() 
    {
        if (frontEmpty())
        {
            return iterator(*this, back_bucks_.head(), false);
        }
        return iterator(*this, top_ix_, true);

    }
    iterator end() 
    {
        if (back_bucks_.empty())
        {
            return iterator(*this, bot_ix_, true);
        }
        return iterator(*this, back_bucks_.tail(), false);
    }

    iterator find(const Key& key)
    {
        int key_ix = top_ix_ + Compare::diff(key - top_);
        if (key_ix >= top_ix_ && key_ix < bot_ix_ &&
            !front_bucks_[key_ix & mask_].empty()
        )
        {
            return  iterator(*this, key_ix, true);
        }
        auto iter = back_bucks_.find(key);
        return  iterator(*this, iter.index(), false);
    }

    value_type getFrontEntry(int ix)
    {
        return
            std::make_pair(top_ + ix - top_ix_, front_bucks_[ix & mask_]);
    }

    value_type getBackEntry(int ix)
    {
        return back_bucks_[ix & mask_]);
    }

    bool isFrontEntryEmpty(int ix)
    {
        return front_bucks_[ix & mask_].empty();
    }

    void resetTop(const Key& key, const T& )
    {
        top_ix_ = 0;
        bot_ix_ = 1;
        front_bucks_[top_ix_].add(val);
        top_ = key;
    }

    void popToFront ()
    {
        while (!back_bucks_.empty())
        {
            const auto& en = back_bucks_.front();
            if (top_ix_ == bot_ix_)
            {
                resetTop(en.first, en.second);
                continue;
            }
            int diff =  Compare::diff(key - top_);
            if (diff < front_bucks_.size())
            {
                int key_ix = top_ix_ + diff;
                front_bucks_[key_ix & mask_].add(val);
                tail_ix_ = key_ix_ + 1;
                continue;
            }
            break;
        }
    }

    /// Push at least num entries from front to back
    void pushToBack (int num)
    {
        while (num > 0 && bot_ix_ > top_ix_)
        {
            --bot_ix_;
            --num;
            const T* bot_val = &front_bucks_[bot_ix_ & mask_];
            while (bot_val->empty())
            {
                --bot_ix_;
                --num;
                bot_val = &front_bucks_[bot_ix_ & mask_];
            }
            Key bot = top_ + bot_ix_ - top_ix_;
            back_bucks_.push_front(bot, *bot_val);
        }
    }

    void add (const Key& key, const T& val)
    {
        if (top_ix_ == bot_ix_)
        {
            popToFront();
        }
        if (top_ix_ == bot_ix_)
        {
            resetTop(key, val);
            return;
        }
        int key_ix = top_ix_ + Compare::diff(key - top);
        if (key_ix < top_ix_)
        {
            int span = key_ix - bot_ix_;
            if (span >= front_bucks_.size())
            {
                pushToBack(span-front_bucks_.size()+1);
            }
            if (top_ix_ == bot_ix_)
            {
                resetTop(key, val);
                return;
            }
            front_bucks_[key_ix & mask_].add(val);
            top_ = key;
            top_ix_ = key_ix_;
        }
        else if (key_ix == top_ix_)
        {
            front_bucks_[top_ix & mask_].add(T);
        }
        else
        {
            int span = key_ix - top_ix;
            if (span < front_bucks_.size())
            {
                front_bucks_[key_ix & mask_].add(val);
                if (key_ix >= tail_ix_)
                {
                    tail_ix_ = key_ix_ + 1;
                }
            }
            else
            {
                back_bucks_.add(key, val);
            }
        }
    }

    void update (const Key& key, const T& val)
    {
        if (top_ix_ == bot_ix_)
        {
            back_bucks_.update(key, val);
            return;
        }
        int key_ix = top_ix_ + Compare::diff(key - top)
        if (key_ix < top_ix)
        {
            return;
        }
        if (key_ix < bot_ix_)
        {
            const T& existing = front_bucks_[key_ix & mask_];
            if (!existing.empty())
            {
                existing.update(val);
                if (key_ix==bot_ix_-1)
                {
                    while (bot_ix_ > top_ix_)
                    {
                        if (!front_bucks_[(bot_ix-1) & mask_].empty())
                        {
                            break;
                        }
                        --bot_ix;
                    }
                }
            }
            return;
        }
        back_bucks_.update(key, val);
    }

};

} // namesapce tf

