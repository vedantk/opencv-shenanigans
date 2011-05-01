CXX = clang++
OPTS = -Wall -O3
LIBS = -lm -lopencv_highgui -lopencv_core -lopencv_imgproc
INCL = -I/usr/include/opencv2
CXXFLAGS =  $(LIBS) $(INCL) $(OPTS)

OBJS = cv-common.hpp.gch color-edges image-capturer fingers
all: $(OBJS)

cv-common.hpp.gch: cv-common.hpp
	$(CXX) -c $^ $(INCL) $(OPTS)

color-edges: color-edges.cpp
image-capturer: image-capturer.cpp
fingers: fingers.cpp

clean:
	rm -f $(OBJS)
