#include "rendertypes.h"


// Start a frame by clearing the screen.
void beginFrame();

// Draw a filled rectangle using engine-defined types.
void drawFilledRect(Color rectColor, Bounds rectBounds);

void drawTexture();

// Show the completed frame.
void presentFrame();