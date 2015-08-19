CFLAGS= -g -Wall -O2

CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

all: $(CPP_OBJ) test

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

test:
	g++ -g   $(CPP_OBJ)  /usr/local/lib/libboost_system.a /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_thread.a /usr/local/lib/libboost_date_time.a /usr/local/lib/libboost_atomic.a  ./libsqlite3.so   -o test

clean:
	rm *~ *.o -f test