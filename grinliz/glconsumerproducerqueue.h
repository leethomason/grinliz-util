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

        // Reads one packet.
        int Consume(DynMemBuf* buf)
        {
            // Upper approach:  100
            // Lower         :  100
            // So not the blocker. Lower is simpler.
#if 0
            if (!consumeQueue.Empty()) {
                return consumeQueue.Pop(buf);
            }
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (queue.Empty())
                    cond.wait(lock);
                queue.Move(&consumeQueue);
                GLASSERT(!consumeQueue.Empty());
            }
            return consumeQueue.Pop(buf);
#else
            std::unique_lock<std::mutex> lock(mutex);
            if (!queue.Empty()) {
                int id = queue.Pop(buf);
                return id;
            }
            cond.wait(lock);
            int id = queue.Pop(buf);
            return id;
#endif
        }

        bool Empty() {
            std::unique_lock<std::mutex> lock(mutex);
            return queue.Empty();
        }

    private:
        std::condition_variable cond;
        std::mutex mutex;
        grinliz::PacketQueue queue;
        //PacketQueue consumeQueue;
    };
}
