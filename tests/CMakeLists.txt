# Existing Phase 1-3 tests. Preserve all current tests except the two obsolete
# Sprite-required assertions, which are updated by phase4_apply.py.
add_executable(engine_tests
    rendering/test_renderer.cpp
    platform/test_platform.cpp
    platform/test_input.cpp
    platform/test_logger.cpp
    core/test_engine.cpp
    ecs/test_component_entity.cpp
    ecs/test_transform_sprite.cpp
    ecs/test_animation_component.cpp
    ecs/test_collision.cpp
    assets/test_asset_manager.cpp
    audio/test_audio_service.cpp
    events/test_event_dispatcher.cpp
    scene/test_scene.cpp
    scene/test_scene_serializer.cpp
    scene/test_scene_serializer_contract.cpp
    ../src/audio/audio_service.cpp
    ../src/rendering/renderer.cpp
    ../src/platform/platform.cpp
    ../src/platform/input.cpp
    ../src/platform/logger.cpp
    ../src/core/engine.cpp
    ../src/core/game.cpp
    ../src/ecs/component.cpp
    ../src/ecs/entity.cpp
    ../src/ecs/sprite_component.cpp
    ../src/ecs/animation_component.cpp
    ../src/ecs/collider_component.cpp
    ../src/ecs/collider_system.cpp
    ../src/events/event_dispatcher.cpp
    ../src/scene/scene.cpp
    ../src/scene/scene_manager.cpp
    ../src/scene/scene_serializer.cpp
    ../src/assets/asset_manager.cpp
)

target_link_libraries(engine_tests PRIVATE
    GTest::gtest_main SDL3::SDL3 nlohmann_json::nlohmann_json)
target_include_directories(engine_tests PRIVATE ${ENGINE_INCLUDE_DIRS})

if(WIN32)
    add_custom_command(TARGET engine_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:engine_tests>)
endif()

add_custom_command(TARGET engine_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:engine_tests>/assets"
    VERBATIM)

include(GoogleTest)
gtest_discover_tests(engine_tests
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    PROPERTIES TIMEOUT 30)

# The editor suite is only built when ENGINE_EDITOR is ON. Tests exercise
# testable state/geometry/commands without creating a visible ImGui window.
if(ENGINE_EDITOR)
    add_executable(editor_tests
        editor/test_scene_viewport_geometry.cpp
        editor/test_editor_toolbar_policy.cpp
        editor/test_editor_dpi_scale.cpp
        editor/test_editor_state.cpp
        editor/test_editor_input.cpp
        editor/test_editor_input_integration.cpp
        editor/test_editor_actions.cpp
        editor/test_inspector_panel.cpp
        editor/test_collision_overlay.cpp
        editor/test_log_buffer.cpp
        editor/test_console_commands.cpp
        editor/test_logger_subscription.cpp
        ../src/editor/editor_state.cpp
        ../src/editor/editor_actions.cpp
        ../src/editor/collider_overlay.cpp
        ../src/editor/log_buffer.cpp
        ../src/editor/console_commands.cpp
        ../src/platform/logger.cpp
        ../src/platform/input.cpp
        ../src/ecs/component.cpp
        ../src/ecs/entity.cpp
        ../src/ecs/sprite_component.cpp
        ../src/ecs/animation_component.cpp
        ../src/ecs/collider_component.cpp
        ../src/ecs/collider_system.cpp
        ../src/events/event_dispatcher.cpp
        ../src/scene/scene.cpp
        ../src/scene/scene_manager.cpp
        ../src/scene/scene_serializer.cpp
        ../src/assets/asset_manager.cpp
        ../src/rendering/renderer.cpp
    )
    target_include_directories(editor_tests PRIVATE
        ${ENGINE_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include/editor)
    target_link_libraries(editor_tests PRIVATE
        GTest::gtest_main SDL3::SDL3 nlohmann_json::nlohmann_json)

    if(WIN32)
        add_custom_command(TARGET editor_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:editor_tests>)
    endif()

    gtest_discover_tests(editor_tests
        DISCOVERY_MODE PRE_TEST
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        PROPERTIES TIMEOUT 30)
endif()
