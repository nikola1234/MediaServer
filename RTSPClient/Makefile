CFLAGS= -g -w -O3 -I/usr/local/include/jrtplib3
C_SRC = $(wildcard *.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

RTSPCLIENTDIR := $(shell pwd)

all:$(C_OBJ) $(CPP_OBJ) RTSPClientSO

.c.o:
	gcc $(CFLAGS) -c $< -o $@
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

RTSPClientSO:
	g++ -g $(C_OBJ) $(CPP_OBJ) $(RTSPCLIENTDIR)/../libs/rtp/libjrtp.a $(RTSPCLIENTDIR)/../libs/boost/libboost_filesystem.a $(RTSPCLIENTDIR)/../libs/boost/libboost_thread.a $(RTSPCLIENTDIR)/../libs/boost/libboost_date_time.a $(RTSPCLIENTDIR)/../libs/boost/libboost_atomic.a -fpic -shared -o ./lib/libRTSPClientSO.so

clean:
	rm *~ *.o -f ./lib/libRTSPClientSO.so
