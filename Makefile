CXX = g++
OPTS = -Wall -g
LIBS = -lm -lopencv_highgui -lopencv_core
CXXFLAGS = -I/usr/include/opencv $(LIBS) $(OPTS)

OBJS = color-edges image-capturer fingers
all: $(OBJS)

color-edges: color-edges.cpp
image-capturer: image-capturer.cpp
fingers: fingers.cpp

clean:
	rm -f $(OBJS)
