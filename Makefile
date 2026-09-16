CXX = g++-14
CXXFLAGS = -std=c++23 -Wall -ggdb -MMD -MP \
           -I./libs/my-lib/include -I./libs -I./src \
           `pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer` \
           -fno-sanitize=address -fno-sanitize=undefined

LIBS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer`
TARGET = apex_ascent
BUILD_DIR = build

SOURCES = ./src/main.cpp ./src/core/GameManager.cpp \
          ./src/objects/cards/Card.cpp ./src/objects/cards/CreatureCard.cpp \
          ./src/objects/cards/SpellCard.cpp \
          ./src/objects/ui/UIRenderUtils.cpp ./src/objects/ui/UIButton.cpp \
          ./src/objects/ui/UITextField.cpp \
          ./libs/my-lib/src/memory-pool.cpp ./src/core/data/CardDatabase.cpp \
          ./src/core/data/ScoreBoard.cpp \
          ./src/core/parsers/CardParser.cpp ./src/core/enums/EnumConverter.cpp \
          ./src/logic/CardFactory.cpp ./src/scenes/SceneBattle.cpp \
          ./src/logic/Board.cpp ./src/scenes/SceneUI.cpp ./src/scenes/SceneMenu.cpp \
          ./src/scenes/ScenePause.cpp \
          ./src/logic/Player.cpp \
          ./src/logic/Opponent.cpp ./src/scenes/SceneCollection.cpp \
          ./src/logic/DeckBuilder.cpp \

OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEPENDS = $(OBJECTS:.o=.d)

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

# Fica no fim de proposito: o make expande o include ao ler o arquivo, entao
# antes de OBJECTS a lista sairia vazia (mudar um header nao recompilava quem o
# inclui) e antes de `all` as regras dos .d roubariam o alvo padrao.
-include $(DEPENDS)