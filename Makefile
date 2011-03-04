CXX = g++
OPTS = -Wall -O3
LIBS = -lm -lopencv_highgui -lopencv_core
CXXFLAGS = -I/usr/include/opencv $(LIBS) $(OPTS)

OBJS = color-edges image-capturer fingers cv-common.hpp.gch
all: $(OBJS)

color-edges: color-edges.cpp
image-capturer: image-capturer.cpp
fingers: fingers.cpp
cv-common.hpp.gch: cv-common.hpp
	$(CXX) -c $^ $(CXXFLAGS)

clean:
	rm -f $(OBJS)
