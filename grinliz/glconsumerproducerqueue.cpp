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

void PacketQueueMT::PushMove(PacketQueue& in)
{
	GLASSERT(!in.Empty());
	std::unique_lock<std::mutex> lock(mutex);
	in.Move(queue);
	cond.notify_one();
}


PacketQueueMT testQueue;

int nProducer = 1;
std::atomic<int64_t> totalTime = 0;
static const int NUM_VALUES = 100'000;

void Produce()
{
	timePoint_t start = Now();
	int nOdds = 0;
	for (int i = 0; i < NUM_VALUES; i++) {
		if (IsOdd(i)) {
			nOdds++;
			testQueue.Push(0, i);
		}
	}
	testQueue.Push(1);
	int64_t d = DeltaMillis(start, Now());
	totalTime += d;
	//printf("Producer nOdd=%d\n", nOdds);
}

void Produce2()
{
	PacketQueue pq;
	int n = 0;
	const int FLUSH = 64;

	timePoint_t start = Now();
	int nOdds = 0;
	for (int i = 0; i < NUM_VALUES; i++) {
		if (IsOdd(i)) {
			nOdds++;
			pq.Push(0, i);
			++n;
			if (n == FLUSH) {
				n = 0;
				testQueue.PushMove(pq);
			}
		}
	}
	testQueue.PushMove(pq);
	testQueue.Push(1);
	int64_t d = DeltaMillis(start, Now());
	totalTime += d;

}

void Consume()
{
	int nOdds = 0;
	int nDone = 0;
	DynMemBuf buf;
	
	timePoint_t start = Now();
	while (true) {
		int id = testQueue.Consume(&buf);
		if (id == 0) {
			int odd = 0;
			buf.Get(&odd);
			nOdds++;
		}
		else if (id == 1) {
			++nDone;
			if (nDone == nProducer)
				break;
		}
	}
	int64_t d = DeltaMillis(start, Now());
	totalTime += d;
	printf("Consumer nOdd=%d\n", nOdds);
}

void grinliz::ConsumerProducerQueueTest(int seed)
{
	// Single producer
	printf("1 producer\n");
	{
		nProducer = 1;
		std::thread t1(Produce);
		std::thread t2(Consume);

		t1.join();
		t2.join();
	}

	totalTime.store(0);

	GLASSERT(testQueue.Empty());
	static constexpr int N = 4;

	printf("4 producers\n");
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

	totalTime.store(0);
	printf("4 producers w/ producer caching\n");
	for (int i = 0; i < N; ++i) {
		nProducer = 4;
		std::thread t1a(Produce2);
		std::thread t1b(Produce2);
		std::thread t1c(Produce2);
		std::thread t1d(Produce2);
		std::thread t2(Consume);

		t2.join();
		t1a.join();
		t1b.join();
		t1c.join();
		t1d.join();
	}
	tt = totalTime.load();
	printf("Ave multi-threaded queue time=%lld millis\n", tt / N);
}
