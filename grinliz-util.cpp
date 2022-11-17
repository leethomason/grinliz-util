#include "grinliz/glconsumerproducerqueue.h"
#include "grinliz/glcontainer.h"
#include "grinliz/glgeometry.h"
#include "grinliz/glstringutil.h"
#include "grinliz/glstringpool.h"
#include "grinliz/gltree.h"
#include "grinliz/glparser.h"
#include "grinliz/glrandom.h"

int CountBits(uint32_t a)
{
	int count = 0;
	for (int i = 0; i < 32; ++i)
		if (a & (1 << i))
			++count;
	return count;
}

bool TestRandom()
{
//	printf("%d %d %d\n", 
//		CountBits(grinliz::Random::Mix(0)), 
//		CountBits(grinliz::Random::Mix(1)), 
//		CountBits(grinliz::Random::Mix(3)));
	printf("%x %x %x\n",
		grinliz::Random::Mix(1),
		grinliz::Random::Mix(2),
		grinliz::Random::Mix(3));
	return true;
}

int main()
{
	TestRandom();
	grinliz::TestContainers();
	grinliz::ConsumerProducerQueueTest(clock());
	grinliz::TestRect();
	grinliz::TestIntersect();
	grinliz::Frustum::Test();
	grinliz::StringPool::Test();
	grinliz::TreeTest();
	grinliz::TestCSV();

	printf("Tests pass.\n");
	return 0;
}

