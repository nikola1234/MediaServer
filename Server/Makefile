
CROSS_COMPILE =
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CXX   = $(CROSS_COMPILE)g++
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP			= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CXX  CPP AR NM
export STRIP OBJCOPY OBJDUMP

CXXFLAGS := -Wall  -O2 -g
CXXFLAGS += -I $(shell pwd)/include  -I $(shell pwd)/../Analysis
CXXFLAGS += $(shell pkg-config --cflags OpenCVPC) -lavformat -lavcodec -lavfilter -lavutil -lswresample -lswscale -lz -D__STDC_CONSTANT_MACROS

LDLIBS   := $(shell pkg-config --libs  OpenCVPC)-lboost_system -lboost_date_time -lboost_filesystem -lboost_thread -lboost_atomic -lpthread -lm

dynamicLIBS     :=  ../RTSPServer/lib/libRTSPServerSO.so  ../RTSPClient/lib/libRTSPClientSO.so  ../Analysis/lib/libAnalysisSO.so  ./lib/libjson.so

export CXXFLAGS LDLIBS

SUBDIR := $(shell pwd)
export SUBDIR

TARGET := demo

obj-y += main.o
obj-y += Camera/
obj-y += File/
obj-y += Log/
obj-y += Net/

all :
	make  -C ./ -f $(SUBDIR)/Makefile.build
	$(CXX)  -o $(TARGET) built-in.o  $(dynamicLIBS) $(LDLIBS) $(CXXFLAGS)

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
