TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

build_pass:CONFIG(debug, debug|release) {
unix: {
    LIBS += "../LEti-engine/OpenGL/Linux/lib/libglfw3.a"
    LIBS += "../LEti-engine/OpenGL/Linux/lib/libGLEW.a"

    INCLUDEPATH += "OpenGL/Linux/include/"
}
else: {
    LIBS += "../LEti-engine/debug/libLEti_Engine.a"
    LIBS += "../LEti-engine/OpenGL/Windows_x64_mingw/lib/libglfw3.a"
    LIBS += "../LEti-engine/OpenGL/Windows_x64_mingw/lib/libglew32.a"

    LIBS += -lopengl32 \
        -luser32 \
        -lgdi32 \
        -lshell32

    INCLUDEPATH += "../LEti-Engine/include/" \
        "../LEti-Engine/" \
        "../LEti-Engine/OpenGL/Windows_x64_mingw/include/" \
        "../LEti-Engine/OpenGL/Windows_x64_mingw/include/" \
}
}


SOURCES += \
Main.cpp \

DISTFILES += \ \
Resources/Font/font_yellow.png \
Resources/Models/colliding_object.mdl \
Resources/Models/debug_frame.mdl \
Resources/Models/flat_co.mdl \
Resources/Models/flat_co_2.mdl \
Resources/Models/flat_colliding_object.mdl \
Resources/Models/flat_cos/flat_co_1.mdl \
Resources/Models/flat_cos/flat_co_2.mdl \
Resources/Models/flat_cos/flat_co_3.mdl \
Resources/Models/flat_indicator_red.mdl \
Resources/Models/intersection_indicator_3d.mdl \
Resources/Models/intersection_point_indicator.mdl \
Resources/Models/pyramid.mdl \
Resources/Models/quad.mdl \
Resources/Models/red_cube.mdl \
Resources/Models/text_field.mdl \
Resources/Shaders/fragment_shader.shader \
Resources/Shaders/vertex_shader.shader \
Resources/Textures/quad.png \
Resources/Textures/textures.mdl \
Resources/Textures/ugly_color.png

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../LEti-Engine/OpenGL/GLFW/lib/ -lglfw3
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../LEti-Engine/OpenGL/GLFW/lib/ -lglfw3
