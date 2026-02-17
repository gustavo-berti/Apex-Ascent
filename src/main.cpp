#include "core/GameManager.hpp"

int main(int argc, char* argv[]) {
    GameManager* game = new GameManager();

    if(game->Initialize("Apex Ascent", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, false)) {
        game->Run();
    }

    delete game;
    return 0;
}