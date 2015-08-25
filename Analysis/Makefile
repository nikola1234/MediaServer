
ANALYZEDIR := $(shell pwd)
CXXFLAGS := -g -w -O3 -I ./
CXXFLAGS += $(shell pkg-config --cflags OpenCVPC)  -lz -D__STDC_CONSTANT_MACROS

#LDLIBS   := $(shell pkg-config --libs  OpenCVPC) 
OPENCVLIBS :=$(ANALYZEDIR)/../libs/opencv/libopencv_contrib.so   $(ANALYZEDIR)/../libs/opencv/libopencv_core.so  $(ANALYZEDIR)/../libs/opencv/libopencv_features2d.so \
				$(ANALYZEDIR)/../libs/opencv/libopencv_flann.so  $(ANALYZEDIR)/../libs/opencv/libopencv_gpu.so  $(ANALYZEDIR)/../libs/opencv/libopencv_highgui.so \
				$(ANALYZEDIR)/../libs/opencv/libopencv_imgproc.so  $(ANALYZEDIR)/../libs/opencv/libopencv_legacy.so  $(ANALYZEDIR)/../libs/opencv/libopencv_ml.so \
				$(ANALYZEDIR)/../libs/opencv/libopencv_nonfree.so  $(ANALYZEDIR)/../libs/opencv/libopencv_objdetect.so  $(ANALYZEDIR)/../libs/opencv/libopencv_ocl.so \
				$(ANALYZEDIR)/../libs/opencv/libopencv_photo.so  $(ANALYZEDIR)/../libs/opencv/libopencv_video.so

CPP_SRC = $(wildcard *.cpp fire/*.cpp human/*.cpp region/*.cpp smoke/*.cpp)
CPP_OBJ = $(patsubst %cpp, %o, $(CPP_SRC))

.PHONY:all clean

all:$(CPP_OBJ) AnalysisSO

.cpp.o:
	g++ $(CXXFLAGS) -c $< -o $@

AnalysisSO:
	g++ -g  $(CPP_OBJ)  -static -lc $(LDLIBS) $(OPENCVLIBS)   -lm  -lrt -shared -o ./lib/libAnalysisSO.so

clean:
	rm $(wildcard *.o fire/*.o human/*.o region/*.o smoke/*.o)  ./lib/libAnalysisSO.so
