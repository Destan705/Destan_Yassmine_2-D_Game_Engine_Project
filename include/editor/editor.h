#pragma once 
#include <SDL3/SDL.h>

class Editor {
  public:
  Editor(SDL_Window* window, SDL_Renderer* renderer);
  ~Editor();
  Editor(const Editor&) = delete; // sign #1: no making a new box as a copy
  Editor& operator=(const Editor&) = delete; // sign #2: no turning an old box into a copy

};