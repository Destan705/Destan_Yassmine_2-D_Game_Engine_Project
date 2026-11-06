#include "editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

Editor::Editor(SDL_Window* window, SDL_Renderer* renderer) { 

IMGUI_CHECKVERSION();
//creates context
ImGui::CreateContext();

//SETUP PLATFORM/BACKEND

//connect gui to SDL input
ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
//A line that starts the drawing backend
ImGui_ImplSDLRenderer3_Init(renderer);

}

Editor::~Editor() { 

    ImGui_ImplSDLRenderer3_Shutdown();

    ImGui_ImplSDL3_Shutdown();

    ImGui::DestroyContext();

}