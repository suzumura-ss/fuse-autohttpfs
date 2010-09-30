# DEBUG_OPT=-g -O0 -fno-inline
CFLAGS=-Wall -O3 -pthread
SRC=autohttpfs.cpp log.cpp curlaccessor.cpp context.cpp remoteattr.cpp \
    cache.cpp dirent.cpp filestat.cpp ext/time_iso8601.cpp
FUSEOPT=`pkg-config fuse --cflags --libs`
CURLOPT=`pkg-config libcurl --cflags --libs`
VERSIONS =-DLIBFUSE_VERSION=\"`pkg-config fuse --modversion`\"
VERSIONS+=-DLIBCURL_VERSION=\"`pkg-config libcurl --modversion`\"

all: autohttpfs

autohttpfs: main.cpp ${SRC} ${SRC:.cpp=.h} int64format.h Makefile
	g++ ${CFLAGS} ${DEBUG_OPT} ${VERSIONS} -o autohttpfs main.cpp ${SRC} ${FUSEOPT} ${CURLOPT}

int64format.h:
	@echo "int main(){return 0;}" > tmp.c
	@gcc tmp.c
	@file a.out | awk '{print $$3}' > bits
	@case "`cat bits`" in \
    64-bit) \
      echo "#define FINT64 \"l\"" > int64format.h ; \
      echo "#define FSIZET \"l\"" >> int64format.h ; \
      ;; \
    32-bit|*) \
	  echo "#define FINT64 \"ll\"" > int64format.h ; \
      echo "#define FSIZET \"\""   >> int64format.h ; \
      ;; \
    esac
	@rm tmp.c a.out bits

install:
	install autohttpfs /usr/local/bin

clean:
	@-rm -f autohttpfs int64format.h
