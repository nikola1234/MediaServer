CFLAGS= -g -w -O3  -I ./inc


CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

LDServerINC := ../RTSPServer/lib/libRTSPServerSO.so
LDClientINC := ../RTSPClient/lib/libRTSPClientSO.so

.PHONY:all clean

all:$(CPP_OBJ) test

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

test:
	g++ -g  $(CPP_OBJ) $(LDServerINC) $(LDClientINC) -lpthread  -lrt -ldl  -o test

clean:
	rm *~ *.o -f test
