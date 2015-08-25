CFLAGS= -g -w -O3 -I/usr/local/include/jrtplib3
C_SRC = $(wildcard *.c CppSQLite3/*.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp baselib/*.cpp CppSQLite3/*.cpp http/*.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

RTSPSERVERDIR := $(shell pwd)

all:$(C_OBJ) $(CPP_OBJ) RTSPServerSO

.c.o:
	gcc $(CFLAGS) -c $< -o $@
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

RTSPServerSO:
	g++ -g $(C_OBJ) $(CPP_OBJ) -L. -static -lc  -lrt  $(RTSPSERVERDIR)/../libs/rtp/libjrtp.a  $(RTSPSERVERDIR)/../libs/boost/libboost_system.a  $(RTSPSERVERDIR)/../libs/boost/libboost_filesystem.a  $(RTSPSERVERDIR)/../libs/boost/libboost_thread.a  $(RTSPSERVERDIR)/../libs/boost/libboost_date_time.a  $(RTSPSERVERDIR)/../libs/boost/libboost_atomic.a -lpthread -fpic -shared -o ./lib/libRTSPServerSO.so

clean:
	rm *~ *.o -f ./lib/libRTSPServerSO.so
