TESTS=cache_test
CFLAGS=-g -O0 -Wall -lgtest `pkg-config fuse --cflags --libs`
HELPER=test_helper.cpp

all: ${TESTS}
	./cache_test
	mtrace UrlStatMap_*.mlog

cache_test: cache_test.cpp ../cache.cpp ${HELPER}
	g++ -o $@ ${CFLAGS} $^

clean:
	@rm -f ${TESTS} *.mlog
