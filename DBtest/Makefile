CFLAGS= -g -w -O3 -I ./CppSQLite3

C_SRC = $(wildcard *.c CppSQLite3/*.c)
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp CppSQLite3/*.cpp )
CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

LDServerINC := ../RTSPServer/lib/libRTSPServerSO.so
LDClientINC := ../RTSPClient/lib/libRTSPClientSO.so

.PHONY:all clean

all:$(C_OBJ) $(CPP_OBJ) test

.c.o:
	gcc $(CFLAGS) -c $< -o $@
	
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

test:
	g++ -g  $(C_OBJ) $(CPP_OBJ) $(LDServerINC) $(LDClientINC) -lpthread  -lrt -ldl  -o test

clean:
	rm *~ *.o -f test
