TESTS=cache_test
CFLAGS=-g -O0 -Wall -lgtest `pkg-config fuse --cflags --libs`
CACHE_EXP=-DCACHE_EXPIRES_SEC=1
HELPER=test_helper.cpp ../int64format.h

all: ${TESTS}
	./cache_test
	mtrace UrlStatMap_*.mlog

cache_test: cache_test.cpp ../cache.cpp ${HELPER}
	g++ -o $@ ${CFLAGS} $^ ${CACHE_EXP}

../int64format.h:
	(cd .. && make int64format.h)

clean:
	@rm -f ${TESTS} *.mlog
