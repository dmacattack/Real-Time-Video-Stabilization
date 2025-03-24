CC=g++
CFLAGS=-I/usr/include/opencv4 -I$(PWD)
LIBS=-lopencv_core -lopencv_calib3d -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_stitching -lopencv_videoio -lopencv_video


# pkg-config for gstreamer
PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig
CFLAGS+= $(shell pkg-config --cflags gstreamer-1.0)
LIBS+= $(shell pkg-config --libs gstreamer-1.0)

CFLAGS+= $(shell pkg-config --cflags gstreamer-app-1.0)
LIBS+= $(shell pkg-config --libs gstreamer-app-1.0)

SOURCES=main.cpp videostab.cpp matsender.cpp
OBJECTS=$(patsubst %.cpp,build/%.o,$(SOURCES))
EXECUTABLE=build/videostab

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

build/%.o: %.cpp
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build