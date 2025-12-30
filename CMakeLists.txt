cmake_minimum_required(VERSION 3.5)

project(Shardis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE(GLOB_RECURSE LibFiles "include/*.h")
add_custom_target(_Shardis_include SOURCES ${LibFiles})
FILE(GLOB_RECURSE Models "Resources/*")
add_custom_target(Shardis_models SOURCES ${Models})

file(GLOB_RECURSE Sources "source/*.cpp")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DL_DEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DL_DEBUG")
# set(CMAKE_CXX_FLAGS "-DL_DEBUG")

if(MSVC)
    add_compile_options(/bigobj)
endif(MSVC)

add_executable(Shardis ${Sources} Main.cpp)

add_subdirectory("${PROJECT_SOURCE_DIR}/L_Utility"
                 "${PROJECT_BINARY_DIR}/L_Utility"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Variable"
                 "${PROJECT_BINARY_DIR}/L_Variable"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/LEti_Engine"
                 "${PROJECT_BINARY_DIR}/LEti_Engine"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Renderer"
                 "${PROJECT_BINARY_DIR}/L_Renderer"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Physics"
                 "${PROJECT_BINARY_DIR}/L_Physics"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Sound"
                 "${PROJECT_BINARY_DIR}/L_Sound"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Engine_Modules_Dump"
                 "${PROJECT_BINARY_DIR}/L_Engine_Modules_Dump"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_Gamedev_Lib"
                 "${PROJECT_BINARY_DIR}/L_Gamedev_Lib"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/L_GUI"
                 "${PROJECT_BINARY_DIR}/L_GUI"
)

target_include_directories(Shardis PUBLIC
                            "${PROJECT_SOURCE_DIR}"
                            "${PROJECT_SOURCE_DIR}/include"
                            "${PROJECT_SOURCE_DIR}/L_Utility/include"
                            "${PROJECT_SOURCE_DIR}/L_Variable/include"
                            "${PROJECT_SOURCE_DIR}/LEti_Engine/include"
                            "${PROJECT_SOURCE_DIR}/LEti_Engine/OpenGL/GLM"
                            "${PROJECT_SOURCE_DIR}/L_Renderer/include"
                            "${PROJECT_SOURCE_DIR}/L_Renderer/OpenGL/include"
                            "${PROJECT_SOURCE_DIR}/L_Physics/include"
                            "${PROJECT_SOURCE_DIR}/L_Sound/include"
                            "${PROJECT_SOURCE_DIR}/L_Engine_Modules_Dump/include"
                            "${PROJECT_SOURCE_DIR}/L_Gamedev_Lib/include"
                            "${PROJECT_SOURCE_DIR}/L_GUI/include"
)


target_link_libraries(Shardis PUBLIC
    L_GUI
    L_Gamedev_Lib
    L_Engine_Modules_Dump
    L_Sound
    L_Physics
    L_Renderer
    LEti_Engine
    L_Variable
    L_Utility
)

add_library(glfw STATIC IMPORTED)
add_library(glew STATIC IMPORTED)
add_library(OpenAL STATIC IMPORTED)

if(MSVC)
    set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglfw3.lib")
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglew32.lib")
else()
    set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglfw3.a")
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglew32.a")
endif(MSVC)

set_target_properties(OpenAL PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/OpenAL32.lib")

target_link_libraries(Shardis PUBLIC
    glfw
    glew
    OpenAL
)

if (WIN32)
    target_link_libraries(Shardis PUBLIC
        opengl32
        user32
        gdi32
        shell32
    )
endif (WIN32)

if (UNIX)
    target_link_libraries(Shardis PUBLIC
        pthread
        GL
        X11
        dl
    )
endif (UNIX)
