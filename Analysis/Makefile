
CXXFLAGS := -g -w -O3 -I ./
CXXFLAGS += $(shell pkg-config --cflags OpenCVPC)  -lz -D__STDC_CONSTANT_MACROS

LDLIBS   := $(shell pkg-config --libs  OpenCVPC)  -lm  -lrt

CPP_SRC = $(wildcard *.cpp fire/*.cpp human/*.cpp region/*.cpp smoke/*.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

all:$(CPP_OBJ) AnalysisSO

.cpp.o:
	g++ $(CXXFLAGS) -c $< -o $@

AnalysisSO:
	g++ -g  $(CPP_OBJ)  -static -lc $(LDLIBS) -shared -o ./lib/libAnalysisSO.so

clean:
	rm $(wildcard *.o fire/*.o human/*.o region/*.o smoke/*.o)  ./lib/libAnalysisSO.so
