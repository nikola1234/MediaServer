CFLAGS= -g -Wall -O2

C_SRC = $(wildcard *.c )
C_OBJ = $(patsubst %c, %o, $(C_SRC))
CPP_SRC = $(wildcard *.cpp )
CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

all:$(C_OBJ) $(CPP_OBJ) DataBaseSO

.c.o:
	gcc $(CFLAGS) -c $< -o $@
	
.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

DataBaseSO:
	g++ -g  $(C_OBJ) $(CPP_OBJ)  /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_date_time.a /usr/local/lib/libboost_atomic.a -fpic -shared   -o ./lib/libDataBaseSO.so

clean:
	rm *~ *.o -f ./lib/libDataBaseSO.so