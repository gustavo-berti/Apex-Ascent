#include "GameManager.hpp"
#include "../scenes/SceneMenu.hpp"
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<std::string, Mix_Chunk*> GameManager::soundEffects;

GameManager::GameManager() {
    window = nullptr;
    renderer = nullptr;
    currentWorld = nullptr;
    backgroundMusic = nullptr;
    isRunning = false;
}

GameManager::~GameManager() { Clean(); }

bool GameManager::IsPointInsideRect(int x, int y, const SDL_Rect &rect) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

bool GameManager::Initialize(const char *title, int x, int y, int width, int height,
                             bool fullscreen) {
    int flags = 0;
    if (fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        // Inicializa SDL_ttf
        if (TTF_Init() != 0) {
            std::cerr << "Falha ao inicializar SDL_ttf: " << TTF_GetError() << std::endl;
        }

        window = SDL_CreateWindow(title, x, y, width, height, flags);
        if (window) {
            std::cout << "Janela criada!" << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fundo preto
            SDL_RenderSetLogicalSize(renderer, width, height);
        }

        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            std::cerr << "Falha ao inicializar o Audio do SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cerr << "Falha ao inicializar o SDL_mixer: " << Mix_GetError() << std::endl;
            return false;
        }
        
        isRunning = true;
        opponent.SetGuardian(false);
        ChangeMusic("assets/audio/music/menu_theme.mp3");
        
        LoadSFX("card_place", "assets/audio/sfx/card_place.wav");
        LoadSFX("card_pull", "assets/audio/sfx/card_pull.wav");
        LoadSFX("card_death", "assets/audio/sfx/card_death.wav");
        LoadSFX("card_change", "assets/audio/sfx/card_change.wav");
        LoadSFX("card_combat", "assets/audio/sfx/card_combat.wav");

        SceneMenu *menu = new SceneMenu(*this);
        menu->Initialize(renderer);
        currentWorld = menu;
        return true;
    } else {
        isRunning = false;
        return false;
    }
}

void GameManager::ChangeScene(GameWorld *newWorld) {
    if (currentWorld) {
        delete currentWorld;
    }
    currentWorld = newWorld;
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

    while (SDL_PollEvent(&event)) {

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

        if (currentWorld != nullptr) {
            currentWorld->HandleInput(event);
        }
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


void GameManager::ChangeMusic(const std::string &filepath) {
    if (backgroundMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }

    if (!filepath.empty()) {
        backgroundMusic = Mix_LoadMUS(filepath.c_str());
        if (backgroundMusic) {
            Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
            Mix_PlayMusic(backgroundMusic, -1);
        } else {
            std::cerr << "[ÁUDIO] Falha ao trocar música: " << Mix_GetError() << std::endl;
        }
    }
}

void GameManager::LoadSFX(const std::string& name, const std::string& filepath) {
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (chunk != nullptr) {
        soundEffects[name] = chunk;
    } else {
        std::cerr << "[ÁUDIO] Falha ao carregar SFX (" << filepath << "): " << Mix_GetError() << std::endl;
    }
}

void GameManager::PlaySFX(const std::string& name) {
    if (soundEffects.find(name) != soundEffects.end()) {
        Mix_PlayChannel(-1, soundEffects[name], 0);
    } else {
        std::cerr << "[ÁUDIO] Efeito sonoro '" << name << "' nao encontrado!" << std::endl;
    }
}

void GameManager::ClearSFX() {
    for (auto& pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }
    soundEffects.clear();
}

void GameManager::Clean() {
    if (currentWorld) {
        delete currentWorld;
        currentWorld = nullptr;
    }

    if (backgroundMusic != nullptr) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }

    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    TTF_Quit();
    SDL_Quit();
    ClearSFX();
    std::cout << "Jogo finalizado." << std::endl;
}