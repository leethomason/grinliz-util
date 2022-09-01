#include "grinliz/glconsumerproducerqueue.h"
#include "grinliz/glcontainer.h"
#include "grinliz/glgeometry.h"
#include "grinliz/glstringutil.h"
#include "grinliz/glstringpool.h"

int main()
{
	grinliz::ConsumerProducerQueueTest(clock());
	grinliz::TestContainers();
	grinliz::Tokenizer::Test();
	grinliz::Rect3F::Test();
	grinliz::TestIntersect();
	grinliz::Frustum::Test();
	grinliz::StringPool::Test();

	printf("Tests pass.\n");
	return 0;
}

