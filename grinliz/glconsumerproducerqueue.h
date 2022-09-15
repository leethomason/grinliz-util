#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include "../grinliz/glcontainer.h"

namespace grinliz {

    void ConsumerProducerQueueTest(int seed);

    // A multi-threaded queue that supports a bunch of producers
    // sending messages to one consumer. (Actually, multi-consumers
    // probably works too, I just can't think of a use case.)
    // Each packet has an integer id and a payload.
    // 
    // push is non-blocking
    // consume is blocking
    //
    class PacketQueueMT
    {
    public:
        PacketQueueMT() {}

        void Push(int id, const void* data, int nBytes);
        void Push(int id) { Push(id, 0, 0); }

        template<class T>
        void Push(int id, const T& data) { Push(id, &data, sizeof(data)); }

        // Moves the 'in' queue to be sent, empties the 'in' queue.
        void PushMove(PacketQueue& in);

        // Reads one packet.
        int Consume(DynMemBuf* buf)
        {
            // Try to use the cache w/o a lock. Failing that,
            // copy everything from the input queue to the cache.
            if (cache.Empty()) {
                std::unique_lock<std::mutex> lock(mutex);
                // Something could have filled the cache while
                // we got the lock. Check again:
                if (cache.Empty() && queue.Empty())
                    cond.wait(lock);
                GLASSERT(!queue.Empty());
                queue.Move(cache);
                GLASSERT(!cache.Empty());
            }
            return cache.Pop(buf);
        }

        bool Empty() {
            if (cache.Empty()) {
                std::unique_lock<std::mutex> lock(mutex);
                return queue.Empty();
            }
            return false;
        }

    private:
        std::condition_variable cond;
        std::mutex mutex;
        grinliz::PacketQueue queue;
        PacketQueue cache;
    };
}
