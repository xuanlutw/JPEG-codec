EXE = decoder

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CXX = g++
CPPFLAGS += -Iinclude
CFLAGS += -Wall -g -O3
LDFLAGS += -Llib
LDLIBS += -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	$(RM) $(OBJ) $(EXE)
