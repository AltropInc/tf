#pragma once

namespace tf {

class RingBuffer
{
	char*       			buffer_ { nullptr };
	uint64_t    			buffer_size_ { 0 };
	uint64_t    			capacity_ { 0 };
    volatile  uint64_t*	    write_pos_ { nullptr };
	volatile  uint64_t*     read_pos_ { nullptr };
	uint64_t                write_pos_cache_ { 0 };
	uint64_t                read_pos_cache_ { 0 };
	char*                   data_start_ { nullptr };
	char*                   data_end_ { nullptr };

  public:
	bool write (const char* buff, uint64_t len)
	{
		uint64_t read_pos = *read_pos_;
		if (*write_pos_ > read_pos)
		{
			uint64_t free_space = capacity_ - *write_pos_ + read_pos;
			if (free_space < len)
			{
				// free space is not sufficent
				return false;
			}
			if (capacity_ - *write_pos_ > len)
			{
				// tail space is sufficent for the new data
				memcpy(data_start_ + *write_pos_, buf, len);
				*write_pos_ += len;
			}
			else
			{
				// wrap
				uint64_t end_sz = capacity_ - *write_pos_;
				if (end_sz>0)
				{
					memcpy(data_start_ + *write_pos_, buf, end_sz);
				}
				memcpy(data_start_, buf + end_sz, len - end_sz)
				*write_pos_ = len - end_sz;
			}
		}
		else
		{
			if (read_pos - *write_pos_ < len + 1)
			{
				return false;
			}
			memcpy(data_start_ + *write_pos_, buf, len);
			*write_pos_ += len;
		}
		return true;
	}

	size_t fetch(struct iovec& iov)
	{
        uint64_t wp = *wirte_pos_;
        size_t data_size;
        if (wp >= *read_pos_)
        {
            data_size =  wp - *read_pos_;
            iov[0].iov_base = iov[1].iov_base = data_start_ + *read_pos_;
            iov[0].iov_len = data_size;
            iov[1].iov_len = 0;
        }
        else
        {
            // data is wrapped
            iov[1].iov_base = data_start_;
            iov[1].iov_len = wp - buffer_start_;
            iov[0].iov_base = data_start_ + *read_pos_;
            iov[0].iov_len = capacity_ - *read_pos_);
            data_size = iov[0].iov_len + iov[1].iov_len;
        }
        new_read_pos_ = wp;
        return data_size;
    }

	size_t read(char* buf,  uint64_t len)
	{
        uint64_t wp = *wirte_pos_;
        size_t data_size;
        if (wp >= *read_pos_)
        {
            data_size =  wp - *read_pos_;
			if (data_size < len)
			{
				return 0;
			}
			memcpy(buf, data_start_ + *read_pos_, len);
			*read_pos_ += len;
			return len;
        }

		// wrapped case
		if (capacity_ - *read_pos_ >= len)
		{
			memcpy(buf, data_start_ + *read_pos_, len);
			*read_pos_ += len;
			if (*read_pos_ >= capacity_)
			{
				*read_pos_ = 0;
			}
			return len;
		}

		uint64_t end_sz = capacity_ - *read_pos_;
		if (end_sz+wp < len)
		{
			return 0;
		}
		memcpy(buf, data_start_ + *read_pos_, end_sz);
		memcpy(buf+end_sz, data_start_, len - end_sz);
		*read_pos_ = len - end_sz;
        return data_size;
    }

    void commit(uint64_t new_read_pos) { *read_pos_ = new_read_pos; }
    void commit() { *read_pos_ = new_read_pos_; }

    size_t size() const
    {
        uint64_t* wp = wirte_pos_;
        if (wp >= *read_pos_)
		{
			return wp - *read_pos_;
		}
        else
		{ 
		    return capacity - wp + *read_pos_;
		}
    }

   size_t capacity() const {return capacity_ - 1;}
     
};
        
        


class RingBuffer
{
	char * buffer_;
	char * buffer_start_;
	char * buffer_end_;
	size_t capacity_;
	size_t* write_pos_;
	size_t* read_pos_;

  public:
    virtual int allocBuffer(size_t size) = 0;
	int initialize(size_t size);

  public:
    RingBuffer (size_t size)
      : size_(1<<log2_ceil(size))
	  , mask_(size_ - 1)
	  , buffer_(size_)
	  , write_pos_(0)
	  , read_pos_ (0)
	{}

	bool write (char* data, size_t length);
	size_t read(char&* data);
};

int RingBuffer::intialize(size_t size)
{
	if (allocateBuffer(size) < 0)
	{
		return -1;
	}
} 

bool RingBuffer::write(char* data, size_t length)
{
	if (write_pos_)
}

}