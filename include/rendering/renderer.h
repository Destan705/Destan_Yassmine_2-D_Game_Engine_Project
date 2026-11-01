#pragma once
#include <SDL3/SDL.h>
#include "rendertypes.h"
#include <vector> 




class Renderer{
    public:
    explicit Renderer( SDL_Window* window);// contructor
    ~Renderer();// destructor
    Renderer(const Renderer&) = delete;             // sign #1: no making a new box as a copy
    Renderer& operator=(const Renderer&) = delete;  // sign #2: no turning an old box into a copy
  
   
  
    // Fills the whole screen with the given color.
    // Call once at the start of every frame, before drawing.

    void ClearScreen(const Color& color);

    // Shows everything drawn this frame on the screen.
    // Call once at the end of every frame, after drawing.

    void Present();

    // Draws a filled rectangle at the given position and size.
    void DrawFilledRect(const Color& color, const Bounds& bounds);

    //load texture from file
    textureHandle LoadTexture(const char* filePath);

    //Draws the loaded texture at the given position and size.
    void DrawTexture(textureHandle textureHandle, const Bounds& bounds);
    

    private:
    SDL_Renderer* renderer=nullptr;
    bool hasWarned = false; // flag to track if the warning has been logged
      std::vector <SDL_Texture*> textures; // the array of textures that the renderer can use to draw
};