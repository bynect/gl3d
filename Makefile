CXX=g++
CXXFLAGS=-g3
CXXLIBS=-lSDL2

SRC=main.cpp
OBJ=$(SRC:.cpp=.o)
BIN=gl3d.bin

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXLIBS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(BIN) $(OBJ)

