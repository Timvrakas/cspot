#ifndef ALSAAUDIOSINK_H
#define ALSAAUDIOSINK_H

#ifdef CSPOT_ENABLE_ALSA_SINK

#include <vector>
#include <fstream>
#include "AudioSink.h"
#include <tinyalsa/pcm.h>
#include <stdio.h>
#include <Task.h>
#include <unistd.h>
#include <memory>
#include <mutex>

#define PCM_DEVICE "default"

template <typename T, int SIZE>
class RingbufferPointer
{
    typedef std::unique_ptr<T> TPointer;

public:
    explicit RingbufferPointer()
    {
        // create objects
        for (int i = 0; i < SIZE; i++)
        {
            buf_[i] = std::make_unique<T>();
        }
    }

    bool push(TPointer &item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (full())
            return false;

        std::swap(buf_[head_], item);

        if (full_)
            tail_ = (tail_ + 1) % max_size_;

        head_ = (head_ + 1) % max_size_;
        full_ = head_ == tail_;

        return true;
    }

    bool pop(TPointer &item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (empty())
            return false;

        std::swap(buf_[tail_], item);

        full_ = false;
        tail_ = (tail_ + 1) % max_size_;

        return true;
    }

    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
        full_ = false;
    }

    bool empty() const
    {
        return (!full_ && (head_ == tail_));
    }

    bool full() const
    {
        return full_;
    }

    int capacity() const
    {
        return max_size_;
    }

    int size() const
    {
        int size = max_size_;

        if (!full_)
        {
            if (head_ >= tail_)
                size = head_ - tail_;
            else
                size = max_size_ + head_ - tail_;
        }

        return size;
    }

private:
    TPointer buf_[SIZE];

    std::mutex mutex_;
    int head_ = 0;
    int tail_ = 0;
    const int max_size_ = SIZE;
    bool full_ = 0;
};

class ALSAAudioSink : public AudioSink, public Task
{
public:
    ALSAAudioSink();
    ~ALSAAudioSink();
    void feedPCMFrames(std::vector<uint8_t> &data);
    void runTask();

private:
    RingbufferPointer<std::vector<uint8_t>, 3> ringbuffer;
    pcm * pcmHandle;
    unsigned int frames;
    int buff_size = 140;
    std::vector<uint8_t> buff;
};
#endif
#endif
