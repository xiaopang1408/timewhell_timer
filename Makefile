VERSION         =
CC                      =g++
DEBUG           =-DUSE_DEBUG
CFLAGS          =-Wall
SOURCES         =$(wildcard ./*.cpp)
INCLUDES        =-I./
LIB_NAMES       =
LIB_PATH        =-L./lib
OBJ                     =$(patsubst %.cpp, %.o, $(SOURCES))
TARGET          =hello

#links
$(TARGET):$(OBJ)
        @mkdir -p output
        $(CC) $(OBJ) $(LIB_PATH) $(LIB_NAMES) -ggdb -o $(TARGET)$(VERSION)
        @rm -rf $(OBJ)

#compile
%.o: %.cpp
        $(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
        @echo "Remove linked and compiled files......"
        rm -rf $(OBJ) $(TARGET) output
