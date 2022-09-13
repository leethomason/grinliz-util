#include "glconsumerproducerqueue.h"
#include "glrandom.h"
#include "glperformance.h"

using namespace grinliz;

void PacketQueueMT::Push(int id, const void* data, int nBytes)
{
	std::unique_lock<std::mutex> lock(mutex);
	queue.Push(id, nBytes, data);
	cond.notify_one();
}

bool IsPrime(int n) {
	if (n == 0 || n == 1 || n == 2) {
		return false;
	}
	for (int i = 2; i <= n / 2; ++i) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

PacketQueueMT testQueue;
static const int N_PRIME = 100'000;
int nProducer = 1;

void Produce()
{
	int nPrimes = 0;
	for (int i = 2; i < N_PRIME; i++) {
		if (IsPrime(i)) {
			nPrimes++;
			testQueue.Push(0, i);
		}
	}
	testQueue.Push(1);
	printf("Producer nPrimes=%d\n", nPrimes);
}

void Consume()
{
	int nPrimes = 0;
	int nDone = 0;
	DynMemBuf buf;
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
	// Multi-producer
	{
		nProducer = 4;
		std::thread t1a(Produce);
		std::thread t1b(Produce);
		std::thread t1c(Produce);
		std::thread t1d(Produce);
		{
			QuickProfile profile("multi-producer");
			std::thread t2(Consume);
			t2.join();
		}

		t1a.join();
		t1b.join();
		t1c.join();
		t1d.join();
	}
}
