cmake_minimum_required(VERSION 3.30)
project(Sandbox)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

# -----------------------------
# Detect Emscripten
# -----------------------------
if(EMSCRIPTEN)
    add_compile_definitions(PLATFORM_WEB)
    set(PLATFORM "Web")
    set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(RAYLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
else()
    add_compile_definitions(PLATFORM_DESKTOP)
    set(PLATFORM "Desktop")
    add_compile_options("-march=native")
endif()

# -----------------------------
# Fetch raylib
# -----------------------------
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

# -----------------------------
# Fetch raygui
# -----------------------------
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
)
FetchContent_MakeAvailable(raygui)

# # -----------------------------
# # Fetch EnTT
# # -----------------------------
# FetchContent_Declare(
#     entt
#     GIT_REPOSITORY https://github.com/skypjack/entt
#     GIT_TAG v3.15.0
# )
# FetchContent_MakeAvailable(entt)

# -----------------------------
# Create executable
# -----------------------------
add_executable(Sandbox src/main.cpp src/raygui.c src/core/Application.cpp src/core/PixelWorld.cpp src/core/Renderer.cpp)
target_link_libraries(Sandbox PRIVATE raylib)

target_include_directories(Sandbox PRIVATE ${raygui_SOURCE_DIR}/src)


# -----------------------------
# Emscripten-specific settings
# -----------------------------
if(EMSCRIPTEN)
    set_target_properties(Sandbox PROPERTIES 
        LINK_FLAGS "-s USE_GLFW=3 -s INITIAL_MEMORY=16777216 -s ASYNCIFY=1 -s ASYNCIFY_IMPORTS=['emscripten_sleep'] --shell-file ${CMAKE_SOURCE_DIR}/index.html"
    )
    file(COPY ${CMAKE_SOURCE_DIR}/index.html DESTINATION ${CMAKE_BINARY_DIR})
endif()
