DEBUG_OPT=-g -O0
CFLAGS=-Wall -O3 -pthread
SRC=autohttpfs.cpp log.cpp curlaccessor.cpp context.cpp remoteattr.cpp cache.cpp
FUSEOPT=`pkg-config fuse --cflags --libs`
CURLOPT=`pkg-config libcurl --cflags --libs`

all: autohttpfs

autohttpfs: main.cpp globals.h ${SRC} ${SRC:.cpp=.h} int64format.h Makefile
	g++ ${CFLAGS} ${DEBUG_OPT} -o autohttpfs main.cpp ${SRC} ${FUSEOPT} ${CURLOPT}

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

clean:
	@-rm -f autohttpfs int64format.h
