
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

SUBDIR := $(shell pwd)
export SUBDIR

CXXFLAGS := -Wall  -O2 -g
CXXFLAGS += -I $(shell pwd)/include  -I $(shell pwd)/../Analysis
CXXFLAGS += $(shell pkg-config --cflags OpenCVPC) -lz -D__STDC_CONSTANT_MACROS

LDLIBS   := -lpthread -lm

opencvLIBS:=$(SUBDIR)/../libs/opencv/libopencv_contrib.so  $(SUBDIR)/../libs/opencv/libopencv_core.so  $(SUBDIR)/../libs/opencv/libopencv_features2d.so \
				$(SUBDIR)/../libs/opencv/libopencv_flann.so $(SUBDIR)/../libs/opencv/libopencv_gpu.so $(SUBDIR)/../libs/opencv/libopencv_highgui.so \
				$(SUBDIR)/../libs/opencv/libopencv_imgproc.so  $(SUBDIR)/../libs/opencv/libopencv_legacy.so  $(SUBDIR)/../libs/opencv/libopencv_ml.so \
				$(SUBDIR)/../libs/opencv/libopencv_nonfree.so  $(SUBDIR)/../libs/opencv/libopencv_objdetect.so  $(SUBDIR)/../libs/opencv/libopencv_ocl.so \
				$(SUBDIR)/../libs/opencv/libopencv_photo.so  $(SUBDIR)/../libs/opencv/libopencv_video.so

ffmpegLibs := $(SUBDIR)/../libs/ffmpeg/libavformat.so.56.25.101 	$(SUBDIR)/../libs/ffmpeg/libavutil.so.54.20.100 	 $(SUBDIR)/../libs/ffmpeg/libavfilter.so.5.11.102 	\
			$(SUBDIR)/../libs/ffmpeg/libavcodec.so.56.26.100  $(SUBDIR)/../libs/ffmpeg/libswresample.so.1.1.100   $(SUBDIR)/../libs/ffmpeg/libswscale.so.3.1.101	

BoostLIBS :=$(SUBDIR)/../libs/boost/libboost_system.a  $(SUBDIR)/../libs/boost/libboost_filesystem.a  $(SUBDIR)/../libs/boost/libboost_thread.a  $(SUBDIR)/../libs/boost/libboost_date_time.a  $(SUBDIR)/../libs/boost/libboost_atomic.a

dynamicLIBS     :=  ../RTSPServer/lib/libRTSPServerSO.so  ../RTSPClient/lib/libRTSPClientSO.so ../Analysis/lib/libAnalysisSO.so  ./lib/libjson.so ./lib/libsqlite3.so

export CXXFLAGS LDLIBS

TARGET := demo

obj-y += main.o
obj-y += Camera/
obj-y += File/
obj-y += Log/
obj-y += Net/
obj-y += Db/

all :
	make  -C ./ -f $(SUBDIR)/Makefile.build
	$(CXX)  -o $(TARGET) built-in.o  $(dynamicLIBS) $(LDLIBS) $(BoostLIBS)  $(opencvLIBS) $(ffmpegLibs) $(CXXFLAGS)

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
