#DEBUG_OPT=-g -O0 -fno-inline
SRC=autohttpfs.cpp log.cpp curlaccessor.cpp context.cpp remoteattr.cpp \
    cache.cpp dirent.cpp proc.cpp procmap.cpp filestat.cpp ext/time_iso8601.cpp
LDFLAGS  =`pkg-config fuse --libs` `pkg-config libcurl --libs`
VERSIONS =-DLIBFUSE_VERSION=\"`pkg-config fuse --modversion`\"
VERSIONS+=-DLIBCURL_VERSION=\"`pkg-config libcurl --modversion`\"
DEFS     =${DEBUG_OPT} ${VERSIONS} `pkg-config fuse --cflags` `pkg-config libcurl --cflags`
CPPFLAGS =-Wall -O3 -pthread ${DEFS}

all: depend autohttpfs

autohttpfs: int64format.h main.cpp ${SRC} ${SRC:.cpp=.o} Makefile
	g++ ${CPPFLAGS} ${LDFLAGS} -o autohttpfs main.cpp ${SRC:.cpp=.o}

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
	@-rm -f autohttpfs make.depends int64format.h *.o ext/*.o *.bak

depend:
	@touch make.depends
	@makedepend ${DEFS} ${SRC} -fmake.depends > /dev/null 2>&1
	@rm -f make.depends.bak

-include make.depend
