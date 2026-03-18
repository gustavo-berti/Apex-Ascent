#include "GameManager.hpp"
#include <iostream>

GameManager::GameManager() {
    window = nullptr;
    renderer = nullptr;
    currentWorld = nullptr;
    isRunning = false;
}

GameManager::~GameManager() {
    Clean();
}

bool GameManager::Initialize(const char* title, int x, int y, int width, int height, bool fullscreen) {
    int flags = 0;
    if (fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        window = SDL_CreateWindow(title, x, y, width, height, flags);
        if (window) {
            std::cout << "Janela criada!" << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fundo preto
            SDL_RenderSetLogicalSize(renderer, width, height);
        }

        isRunning = true;
        
        currentWorld = new GameWorld();
        return true;
    } else {
        isRunning = false;
        return false;
    }
}

void GameManager::Run() {
    Uint32 lastTime = SDL_GetTicks();

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        HandleEvents();
        Update();
        
        if (currentWorld) currentWorld->Update(dt);
        
        Render();
    }
}

void GameManager::HandleEvents() {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    switch (event.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_F11) {
                ToggleFullscreen();
            }

            if (event.key.keysym.sym == SDLK_ESCAPE) {
                Uint32 flags = SDL_GetWindowFlags(window);

                if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) {
                    ToggleFullscreen(); 
                } else {
                    isRunning = false; 
                }
            }
            break;
        default:
            break;
    }
}

void GameManager::Update() {}

void GameManager::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (currentWorld) {
        currentWorld->Render(renderer);
    }

    SDL_RenderPresent(renderer);
}

void GameManager::ToggleFullscreen() {
    Uint32 flags = SDL_GetWindowFlags(window);

    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_SetWindowFullscreen(window, 0);
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

void GameManager::Clean() {
    if (currentWorld) {
        delete currentWorld;
        currentWorld = nullptr;
    }
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    std::cout << "Jogo finalizado." << std::endl;
}