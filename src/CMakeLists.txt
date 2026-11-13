
add_executable(game_engine
    main.cpp

    core/engine.cpp
    core/game.cpp

    platform/input.cpp
    platform/platform.cpp
    platform/logger.cpp

    rendering/renderer.cpp

    ecs/component.cpp
    ecs/entity.cpp
    ecs/sprite_component.cpp
    ecs/animation_component.cpp
    ecs/collider_component.cpp
    ecs/collider_system.cpp

    events/event_dispatcher.cpp

    scene/scene.cpp
    scene/scene_manager.cpp
    scene/scene_serializer.cpp

    assets/asset_manager.cpp

    audio/audio_service.cpp

)

target_link_libraries(game_engine PRIVATE SDL3::SDL3)
target_link_libraries(game_engine PRIVATE nlohmann_json::nlohmann_json)

if (ENGINE_EDITOR)
target_link_libraries(game_engine PRIVATE imgui_lib)
target_compile_definitions(game_engine PRIVATE ENGINE_EDITOR)
target_sources(game_engine PRIVATE editor/editor.cpp editor/editor_state.cpp editor/editor_actions.cpp editor/inspector_panel.cpp editor/hierarchy_panel.cpp)
target_include_directories(game_engine PRIVATE ${PROJECT_SOURCE_DIR}/include/editor)
endif()

target_include_directories(game_engine PRIVATE
    ${ENGINE_INCLUDE_DIRS}
)

# Copy SDL3.dll next to the executable on Windows.
if(WIN32)
    add_custom_command(
        TARGET game_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:game_engine>
    )
endif()

# Copy game assets next to the executable.
add_custom_command(
    TARGET game_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:game_engine>/assets"
    VERBATIM
)
