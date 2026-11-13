#include "editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "platform.h"

Editor::Editor(SDL_Window* window, SDL_Renderer* renderer) { 

sdlRenderer = renderer;

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

    // Starts a new ImGui frame. Call once per frame, after input is processed
    // and before any editor windows are built.
void Editor::beginFrame(){
        ImGui_ImplSDLRenderer3_NewFrame(); //the drawing backend
        ImGui_ImplSDL3_NewFrame(); // the input backend
        ImGui::NewFrame(); //ImGui itself
}

    // Finishes the ImGui frame and draws the editor windows on top of the scene.
    // Call after the scene is drawn and before Present().
    
void  Editor::render(){
        ImGui::Render(); // turns everyhting in to a list of shapes to draw
        ImGui_ImplSDLRenderer3_RenderDrawData( ImGui::GetDrawData(), sdlRenderer);

}

void Editor::drawPanels(EditorState& editorState,Scene& scene,const AssetManager& assetManager){
    hierarchyPanel.draw(editorState, &scene);
    inspectorPanel.draw(editorState,scene, assetManager);
}


