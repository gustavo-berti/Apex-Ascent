CXX = g++
CXXFLAGS = -std=c++23 -Wall -ggdb -I./libs/my-lib/include -I./src `pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer`
LIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer`
TARGET = apex_ascent
SOURCES = ./src/main.cpp ./src/core/GameManager.cpp ./src/core/GameWorld.cpp ./libs/my-lib/src/memory-pool.cpp

all:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)
	@echo "Build complete! Execute com ./$(TARGET)"
clean:
	rm -f $(TARGET)