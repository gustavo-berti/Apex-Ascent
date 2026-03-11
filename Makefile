CXX = g++
CXXFLAGS = -std=c++23 -Wall -ggdb -I./libs/my-lib/include -I./libs -I./src `pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer`
LIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer`
TARGET = apex_ascent
SOURCES = ./src/main.cpp ./src/core/GameManager.cpp ./src/core/GameWorld.cpp \
          ./src/objects/Card.cpp ./src/objects/CreatureCard.cpp ./src/objects/SpellCard.cpp \
          ./libs/my-lib/src/memory-pool.cpp ./src/logic/CardDatabase.cpp

all:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)
	@echo "Build complete! Execute com ./$(TARGET)"
clean:
	rm -f $(TARGET)