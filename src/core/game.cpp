#include "game.h"
#include "input.h"
#include "render_api.h"

#include <array>
#include <iostream>

namespace {
constexpr std::array<float, 2> catStartPosition = {100.0f, 250.0f};
constexpr std::array<float, 2> chefStartPosition = {600.0f, 250.0f};
std::array<float, 2> catPosition = catStartPosition;
std::array<float, 2> chefPosition = chefStartPosition;

constexpr float objectSize = 100.0f;
constexpr float movementSpeed = 200.0f;

const Color catColor{0, 0, 255, 255};  // Blue
const Color chefColor{255, 0, 0, 255}; // Red

void updatePosition(std::array<float, 2>& position, float delta_time, Key left, Key right, Key up,
                    Key down, const char* objectName) {
    const float movementDistance = movementSpeed * delta_time;

    if (isKeyHeld(left)) {
        position[0] -= movementDistance;
        std::cout << objectName << " left\n";
    } else if (isKeyHeld(right)) {
        position[0] += movementDistance;
        std::cout << objectName << " right\n";
    } else if (isKeyHeld(up)) {
        position[1] -= movementDistance;
        std::cout << objectName << " up\n";
    } else if (isKeyHeld(down)) {
        position[1] += movementDistance;
        std::cout << objectName << " down\n";
    }
}
} // namespace
void initializeGame() {
    catPosition = catStartPosition;
    chefPosition = chefStartPosition;
}

void updateGame(float delta_time) {
    updatePosition(catPosition, delta_time, Key::A, Key::D, Key::W, Key::S, "Cat");

    updatePosition(chefPosition, delta_time, Key::Left, Key::Right, Key::Up, Key::Down, "Chef");
}

void renderGame() {
    drawFilledRect(catColor, Bounds{catPosition[0], catPosition[1], objectSize, objectSize});

    drawFilledRect(chefColor, Bounds{chefPosition[0], chefPosition[1], objectSize, objectSize});
}