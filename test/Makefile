TESTS=cache_test filestat_test
CPPFLAGS=-g -O0 -Wall -lgtest `pkg-config fuse --cflags --libs`
CACHE_EXP=-DCACHE_EXPIRES_SEC=1
HELPER=test_helper.cpp ../int64format.h

all: test mcheck

test: ${TESTS}
	@rm -f *.mlog
	@for I in ${TESTS}; do \
    echo "Testing: $$I"; \
    export TZ="JST-9"; \
    ./$$I || exit; done

mcheck:
	@for I in *.mlog ; do echo "`mtrace $$I` - $$I"; done

cache_test: cache_test.cpp ../cache.cpp ${HELPER}
	g++ -o $@ ${CPPFLAGS} $^ ${CACHE_EXP}

filestat_test: filestat_test.cpp ../filestat.cpp ../ext/time_iso8601.cpp ${HELPER}
	g++ -o $@ ${CPPFLAGS} $^ ${CACHE_EXP}

../int64format.h:
	(cd .. && make int64format.h)

clean:
	@rm -f ${TESTS} *.mlog
