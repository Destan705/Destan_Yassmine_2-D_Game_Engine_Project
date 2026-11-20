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

target_link_libraries(game_engine PRIVATE SDL3::SDL3 nlohmann_json::nlohmann_json)

if(ENGINE_EDITOR)
    target_compile_definitions(game_engine PRIVATE ENGINE_EDITOR)
    target_compile_definitions(game_engine PRIVATE ENGINE_EDITOR_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
    target_link_libraries(game_engine PRIVATE imgui_lib)
    target_sources(game_engine PRIVATE
        editor/editor.cpp
        editor/editor_state.cpp
        editor/editor_actions.cpp
        editor/inspector_panel.cpp
        editor/hierarchy_panel.cpp
        editor/scene_file_panel.cpp
        editor/collider_overlay.cpp
        editor/log_buffer.cpp
        editor/console_panel.cpp
        editor/console_commands.cpp
        editor/engine_stats.cpp
        editor/stats_panel.cpp
    )
    target_include_directories(game_engine PRIVATE ${PROJECT_SOURCE_DIR}/include/editor)
endif()

target_include_directories(game_engine PRIVATE ${ENGINE_INCLUDE_DIRS})

if(WIN32)
    add_custom_command(TARGET game_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:game_engine>)
endif()

add_custom_command(TARGET game_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:game_engine>/assets"
    VERBATIM)
