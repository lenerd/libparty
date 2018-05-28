// MIT License
//
// Copyright (c) 2018 Lennart Braun
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

#include <future>
#include <boost/asio/post.hpp>
#include "threading.hpp"

std::pair<size_t, size_t> get_interval(size_t num_stuff, size_t num_threads, size_t thread_id)
{
    auto base_len = num_stuff / num_threads;
    auto rest = num_stuff % num_threads;
    auto start = thread_id * base_len;
    start += thread_id < rest ? thread_id : rest;
    auto end = (thread_id + 1) * base_len;
    end += (thread_id + 1) < rest ? (thread_id + 1) : rest;
    return {start, end};
}

void compute(boost::asio::thread_pool& thread_pool, size_t num_stuff, size_t num_threads, std::function<void(size_t index)> func)
{
    std::vector<std::promise<void>> promises(num_threads);
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id)
    {
        auto interval = get_interval(num_stuff, num_threads, thread_id);
        boost::asio::post(thread_pool, [interval, func, &promises, thread_id]
            {
                for (size_t i = interval.first; i < interval.second; ++i)
                {
                    func(i);
                }
                promises[thread_id].set_value();
            });
    }
    for (size_t thread_id = 0; thread_id < num_threads; ++thread_id)
    {
        promises[thread_id].get_future().get();
    }
}
