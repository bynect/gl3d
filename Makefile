CXX=g++
CXXFLAGS=-g3
CXXLIBS=-lSDL2 -lSDL2_image

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
BIN=gl3d.bin

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXLIBS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(BIN) $(OBJ)

