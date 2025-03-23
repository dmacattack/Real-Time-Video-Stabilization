CC=g++
CFLAGS=-I/usr/include/opencv4 -I$(PWD)
LIBS=-lopencv_core -lopencv_calib3d -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_stitching -lopencv_videoio -lopencv_video

SOURCES=main.cpp videostab.cpp
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