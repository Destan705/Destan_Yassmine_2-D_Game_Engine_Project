
add_executable(engine_tests
    # Tests
    rendering/test_renderer.cpp

    platform/test_platform.cpp
    platform/test_input.cpp
    platform/test_logger.cpp

    core/test_engine.cpp

    ecs/test_component_entity.cpp
    ecs/test_transform_sprite.cpp

    events/test_event_dispatcher.cpp

    scene/test_scene.cpp

    # Engine source files
    ../src/rendering/renderer.cpp

    ../src/platform/platform.cpp
    ../src/platform/input.cpp
    ../src/platform/logger.cpp

    ../src/core/engine.cpp
    ../src/core/game.cpp

    ../src/ecs/component.cpp
    ../src/ecs/entity.cpp
    ../src/ecs/sprite.cpp
    ../src/ecs/animationComponent.cpp

    ../src/events/event_dispatcher.cpp

    ../src/scene/scene.cpp
    ../src/scene/scene_manager.cpp
)

target_link_libraries(engine_tests PRIVATE
    GTest::gtest_main
    SDL3::SDL3
)

target_include_directories(engine_tests PRIVATE
    ${ENGINE_INCLUDE_DIRS}
)

# Test game behavior without SDL by supplying fake
# input and drawing functions.
add_executable(game_tests
    core/test_game.cpp
    ../src/core/game.cpp
)

target_link_libraries(game_tests PRIVATE
    GTest::gtest_main
)

target_include_directories(game_tests PRIVATE
    ${ENGINE_INCLUDE_DIRS}
)

# Copy SDL3.dll next to the test executable on Windows.
if(WIN32)
    add_custom_command(
        TARGET engine_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:engine_tests>
    )
endif()

# Copy assets for engine tests.
add_custom_command(
    TARGET engine_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:engine_tests>/assets"
    VERBATIM
)

include(GoogleTest)

gtest_discover_tests(
    engine_tests
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    PROPERTIES TIMEOUT 30
)
