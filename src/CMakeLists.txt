
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
    ecs/sprite.cpp
    ecs/animationComponent.cpp

    events/event_dispatcher.cpp

    scene/scene.cpp
    scene/scene_manager.cpp
)

target_link_libraries(game_engine PRIVATE SDL3::SDL3)

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
