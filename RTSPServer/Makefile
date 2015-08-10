CFLAGS= -g -w -O3 -I/usr/local/include/jrtplib3
C_SRC = $(wildcard *.c CppSQLite3/*.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp baselib/*.cpp CppSQLite3/*.cpp http/*.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

LDClientINC := ../RTSPClient/lib
.PHONY:all clean

all:$(C_OBJ) $(CPP_OBJ) RTSPServerSO

.c.o:
	gcc $(CFLAGS) -c $< -o $@
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

RTSPServerSO:
	g++ -g $(C_OBJ) $(CPP_OBJ) -L $(LDClientINC) -lRTSPClientSO -static -lc  -lrt /usr/local/lib/libboost_system.a /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_date_time.a /usr/local/lib/libboost_atomic.a -lpthread -ljrtp -fpic -shared -o ./lib/libRTSPServerSO.so

clean:
	rm *~ *.o -f
