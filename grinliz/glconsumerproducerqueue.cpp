#include "glconsumerproducerqueue.h"
#include "glrandom.h"
#include "glperformance.h"
#include "glmath.h"

using namespace grinliz;

void PacketQueueMT::Push(int id, const void* data, int nBytes)
{
	std::unique_lock<std::mutex> lock(mutex);
	queue.Push(id, nBytes, data);
	cond.notify_one();
}

PacketQueueMT testQueue;
static const int N_PRIME = 10'000;
int nProducer = 1;
std::atomic<int64_t> totalTime = 0;

void Produce()
{
	timePoint_t start = Now();
	int nPrimes = 0;
	for (int i = 2; i < N_PRIME; i++) {
		if (IsPrime(i)) {
			nPrimes++;
			testQueue.Push(0, i);
		}
	}
	testQueue.Push(1);
	int64_t d = DeltaMillis(start, Now());
	totalTime += d;
	printf("Producer nPrimes=%d\n", nPrimes);
}

void Consume()
{
	int nPrimes = 0;
	int nDone = 0;
	DynMemBuf buf;
	
	timePoint_t start = Now();
	while (true) {
		int id = testQueue.Consume(&buf);
		if (id == 0) {
			int prime = 0;
			buf.Get(&prime);
			nPrimes++;
		}
		else if (id == 1) {
			++nDone;
			if (nDone == nProducer)
				break;
		}
	}
	int64_t d = DeltaMillis(start, Now());
	totalTime += d;
	printf("Consumer nPrimes=%d\n", nPrimes);
}

void grinliz::ConsumerProducerQueueTest(int seed)
{
#if 0
	// Single producer
	{
		nProducer = 1;
		std::thread t1(Produce);
		std::thread t2(Consume);

		t1.join();
		t2.join();
	}
#endif
	GLASSERT(testQueue.Empty());
	static constexpr int N = 10;

	// Multi-producer
	for (int i = 0; i < N; ++i) {
		nProducer = 4;
		std::thread t1a(Produce);
		std::thread t1b(Produce);
		std::thread t1c(Produce);
		std::thread t1d(Produce);
		std::thread t2(Consume);

		t2.join();
		t1a.join();
		t1b.join();
		t1c.join();
		t1d.join();

	}
	int64_t tt = totalTime.load();
	printf("Ave multi-threaded queue time=%lld millis\n", tt/N);
}
