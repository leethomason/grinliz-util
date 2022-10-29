#include "grinliz/glconsumerproducerqueue.h"
#include "grinliz/glcontainer.h"
#include "grinliz/glgeometry.h"
#include "grinliz/glstringutil.h"
#include "grinliz/glstringpool.h"
#include "grinliz/gltree.h"
#include "grinliz/glparser.h"

int main()
{
	//grinliz::TestContainers();
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

