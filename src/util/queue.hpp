// MIT License
//
// Copyright (c) 2017 Lennart Braun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>

/**
 * Locked queue for elements of type T
 */
template <typename T>
class Queue
{
public:

    /**
     * Adds a new element to the queue.
     */
    void enqueue(const T& item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }

    /**
     * Receives an element from the queue.
     */
    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            cv_.wait(lock, [this] { return !this->queue_.empty(); });
        }
        auto item = queue_.front();
        queue_.pop();
        lock.unlock();
        return item;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // QUEUE_HPP
