#pragma once 
#include <optional>
#include <cstdint>
#include "editor_state.h"
#include "scene.h"
#include "asset_manager.h"


class InspectorPanel{

    private:

    float editX = 0.0f;

    float editY = 0.0f;

    float editHeight=0.0f;

    float editWidth = 0.0f;

    float editColliderOffsetX = 0.0f;

    float editColliderOffsetY = 0.0f;

    float editColliderWidth = 0.0f;

    float editColliderHeight = 0.0f;

    bool editColliderEnabled = true;

    std::optional<std::uint64_t> lastSelectedId;

    std::uint64_t lastSceneGeneration = 0;

    bool lastPaused = true;

    public:

    void draw(EditorState& editorState,Scene& scene,const AssetManager& assetManager );
};