CXX = g++
CXXFLAGS = -std=c++23 -Wall -ggdb -MMD -MP \
           -I./libs/my-lib/include -I./libs -I./src \
           `pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer` \
           -fno-sanitize=address -fno-sanitize=undefined

DEPENDS = $(OBJECTS:.o=.d)
-include $(DEPENDS)
LIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer`
TARGET = apex_ascent
BUILD_DIR = build

SOURCES = ./src/main.cpp ./src/core/GameManager.cpp \
          ./src/objects/cards/Card.cpp ./src/objects/cards/CreatureCard.cpp \
          ./src/objects/cards/SpellCard.cpp \
          ./libs/my-lib/src/memory-pool.cpp ./src/core/data/CardDatabase.cpp \
          ./src/core/parsers/CardParser.cpp ./src/core/enums/EnumConverter.cpp \
          ./src/logic/CardFactory.cpp ./src/logic/Board.cpp ./src/scenes/SceneBattle.cpp \
          ./src/logic/Player.cpp

OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

all: $(TARGET)

rebuild: clean all

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build complete! Execute com ./$(TARGET)"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)