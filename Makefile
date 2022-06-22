SRC=main.cpp
OBJ=$(SRC:.cpp=.o)
BIN=gl3d.bin

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) -lSDL2 -o $@ $<

%.o: %.cpp
	$(CXX) -o $@ -c $<

clean:
	rm -f $(BIN) $(OBJ)

