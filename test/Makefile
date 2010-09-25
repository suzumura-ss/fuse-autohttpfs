TESTS=cache_test
CFLAGS=-g -O3 -Wall -lgtest `pkg-config fuse --cflags --libs`
CACHE_EXP=-DCACHE_EXPIRES_SEC=1
HELPER=test_helper.cpp ../int64format.h

all: test mcheck

test: ${TESTS}
	@rm -f *.mlog
	./cache_test

mcheck:
	@for I in *.mlog ; do echo "`mtrace $$I` - $$I"; done

cache_test: cache_test.cpp ../cache.cpp ${HELPER}
	g++ -o $@ ${CFLAGS} $^ ${CACHE_EXP}

../int64format.h:
	(cd .. && make int64format.h)

clean:
	@rm -f ${TESTS} *.mlog
