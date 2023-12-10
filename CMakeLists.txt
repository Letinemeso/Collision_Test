cmake_minimum_required(VERSION 3.5)

project(Collision_Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB_RECURSE LibFiles "include/*.h")
add_custom_target(Collision_Test_include SOURCES ${LibFiles})


add_executable(Collision_Test
    Main.cpp
)

add_subdirectory("${PROJECT_SOURCE_DIR}/../L_Utility"
                 "${PROJECT_BINARY_DIR}/../L_Utility"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/../L_Variable"
                 "${PROJECT_BINARY_DIR}/../L_Variable"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/../LEti_Engine"
                 "${PROJECT_BINARY_DIR}/../LEti_Engine"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/../L_Renderer"
                 "${PROJECT_BINARY_DIR}/../L_Renderer"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/../L_Physics"
                 "${PROJECT_BINARY_DIR}/../L_Physics"
)
add_subdirectory("${PROJECT_SOURCE_DIR}/../L_Gamedev_Lib"
                 "${PROJECT_BINARY_DIR}/../L_Gamedev_Lib"
)

target_include_directories(Collision_Test PUBLIC
                            "${PROJECT_SOURCE_DIR}"
                            "${PROJECT_SOURCE_DIR}/../L_Utility/include"
                            "${PROJECT_SOURCE_DIR}/../L_Variable/include"
                            "${PROJECT_SOURCE_DIR}/../LEti_Engine/include"
                            "${PROJECT_SOURCE_DIR}/../LEti_Engine/OpenGL/GLM"
                            "${PROJECT_SOURCE_DIR}/../L_Renderer/include"
                            "${PROJECT_SOURCE_DIR}/../L_Physics/include"
                            "${PROJECT_SOURCE_DIR}/../L_Gamedev_Lib/include"
)


target_include_directories(Collision_Test PUBLIC "${PROJECT_SOURCE_DIR}/../L_Renderer/OpenGL/include" )


target_link_libraries(Collision_Test PUBLIC
    L_Gamedev_Lib
    L_Physics
    L_Renderer
    LEti_Engine
    L_Variable
    L_Utility
)

add_library(glfw STATIC IMPORTED)
add_library(glew STATIC IMPORTED)

set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglfw3.a")
set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../Third_Party/libglew32.a")

target_link_libraries(Collision_Test PUBLIC
    glfw
    glew
)

if (WIN32)
    target_link_libraries(Collision_Test PUBLIC
        opengl32
        user32
        gdi32
        shell32
    )
endif (WIN32)

if (UNIX)
    target_link_libraries(Collision_Test PUBLIC
        pthread
        GL
        X11
        dl
    )
endif (UNIX)
