#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include "../grinliz/glcontainer.h"

namespace grinliz {

    template<typename T>
    class ConsumerProducerQueue
    {
    public:
        ConsumerProducerQueue() {}

        void push(const T& request)
        {
            std::unique_lock<std::mutex> lock(mutex);
            cpq.push(request);
            cond.notify_one();
        }

        void consume(T* request)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!cpq.empty()) {
                *request = cpq.front();
                cpq.pop();
                return;
            }
            cond.wait(lock);
            *request = cpq.front();
            cpq.pop();
        }

    private:
        std::condition_variable cond;
        std::mutex mutex;
        std::queue<T> cpq;
    };

    void ConsumerProducerQueueTest(int seed);

    class PacketCPQueue
    {
    public:
        PacketCPQueue() {}

        void push(int id)
        {
            std::unique_lock<std::mutex> lock(mutex);
            queue.Push(id, nullptr);
            cond.notify_one();
        }

        template<class T>
        void push(int id, const T& data)
        {
            GLASSERT(id >= 0 && id < 10000);    // sanity check
            std::unique_lock<std::mutex> lock(mutex);
            queue.Push(id, data);
            cond.notify_one();
        }

        void push(int id, int n, const int* size, const void** data)
        {
            GLASSERT(id >= 0 && id < 10000);    // sanity check
            std::unique_lock<std::mutex> lock(mutex);
            queue.Push(id, n, size, data);
            cond.notify_one();
        }

        int consume(int bufferSize, int* size, void* data)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!queue.Empty()) {
                int id = queue.Pop(bufferSize, size, data);
                return id;
            }
            cond.wait(lock);
            int id = queue.Pop(bufferSize, size, data);
            return id;
        }

        int consume(DynMemBuf* buf)
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!queue.Empty()) {
                int id = queue.Pop(buf);
                return id;
            }
            cond.wait(lock);
            int id = queue.Pop(buf);
            return id;
        }

    private:
        std::condition_variable cond;
        std::mutex mutex;
        grinliz::PacketQueue queue;
    };
}
