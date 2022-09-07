#include "grinliz/glconsumerproducerqueue.h"
#include "grinliz/glcontainer.h"
#include "grinliz/glgeometry.h"
#include "grinliz/glstringutil.h"
#include "grinliz/glstringpool.h"
#include "grinliz/gltree.h"

int main()
{
	grinliz::ConsumerProducerQueueTest(clock());
	grinliz::TestContainers();
	grinliz::Tokenizer::Test();
	grinliz::TestRect();
	grinliz::TestIntersect();
	grinliz::Frustum::Test();
	grinliz::StringPool::Test();
	grinliz::TreeTest();

	printf("Tests pass.\n");
	return 0;
}

