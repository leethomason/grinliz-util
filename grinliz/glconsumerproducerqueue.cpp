#include "glconsumerproducerqueue.h"
#include "glrandom.h"

using namespace grinliz;

PacketCPQueue testQueue;
static const int N_PUSH = 5000;

struct TestMsg0 { int x; };
struct TestMsg1 { static const int N = 37; uint8_t data[N]; };
struct TestMsg2 { int x, y; };

void Produce()
{
	int nBeforeDelay = 1111;
	for (int i = 0; i < N_PUSH; ++i) {
		if ((i & 1) == 0) {
			TestMsg0 t = { 37 };
			testQueue.push(0, t);
		}
		else {
			TestMsg1 t;
			for (int j = 0; j < TestMsg1::N; ++j) t.data[j] = j;
			testQueue.push(1, t);
		}
		if (nBeforeDelay == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			nBeforeDelay = 997;
		}
		nBeforeDelay--;
	}
}

void Produce2()
{
	for (int i = 0; i < N_PUSH/2; ++i) {
		if ((i & 1) == 0) {
			TestMsg0 t = { 37 };
			testQueue.push(0, t);
		}
		else {
			TestMsg2 t = { 7, 9 };
			testQueue.push(2, t);
		}
	}
}

void Consume()
{
	int nBeforeDelay = 2000;

	for (int i = 0; i < N_PUSH; ++i) {
		int size = 0;
		if ((i & 1) == 0) {
			TestMsg0 t;
			testQueue.consume(sizeof(t), &size, &t);
			GLASSERT(size == sizeof(t));
			GLASSERT(t.x == 37);
		}
		else {
			TestMsg1 t;
			testQueue.consume(sizeof(t), &size, &t);
			GLASSERT(size == sizeof(t));
			for (int j = 0; j < TestMsg1::N; ++j) GLASSERT(t.data[j] == j);
		}
		--nBeforeDelay;
		if (nBeforeDelay == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			nBeforeDelay = 1777;
		}
	}
}

void Consume2()
{
	int n0 = 0;
	int n2 = 0;
	uint8_t buf[64];

	for (int i = 0; i < N_PUSH; ++i) {
		int size;
		int id = testQueue.consume(64, &size, buf);

		if (id == 0) {
			GLASSERT(size == sizeof(TestMsg0));
			const TestMsg0* msg = (const TestMsg0*)buf;
			GLASSERT(msg->x == 37);
			++n0;
		}
		else {
			GLASSERT(size == sizeof(TestMsg2));
			const TestMsg2* msg = (const TestMsg2*)buf;
			GLASSERT(msg->x == 7);
			GLASSERT(msg->y == 9);
			++n2;
		}
	}
	GLASSERT(n0 == N_PUSH / 2);
	GLASSERT(n2 == N_PUSH / 2);
}


void grinliz::ConsumerProducerQueueTest(int seed)
{
	// Single producer
	{
		std::thread t1(Produce);
		std::thread t2(Consume);

		t1.join();
		t2.join();
	}
	// Multi-producer
	{
		std::thread t1a(Produce2);
		std::thread t1b(Produce2);
		std::thread t2(Consume2);

		t1a.join();
		t1b.join();
		t2.join();
	}
}
