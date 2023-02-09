TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

unix {
    LIBS += "../Utility/libUtility.a"
    LIBS += "../L_Variable/libL_Variable.a"
    LIBS += "../LEti_Engine/libLEti_Engine.a"

    LIBS += "../../LEti_Engine/OpenGL/Linux/lib/libglfw3.a"
    LIBS += "../../LEti_Engine/OpenGL/Linux/lib/libGLEW.a"

    LIBS += -lpthread -lGL -lX11 -ldl

    INCLUDEPATH += "../LEti_Engine/OpenGL/Linux/include/"
}


win32 {
    LIBS += "../Utility/debug/libUtility.a"
    LIBS += "../L_Variable/debug/libL_Variable.a"
    LIBS += "../LEti_Engine/debug/libLEti_Engine.a"

    LIBS += "../LEti_Engine/OpenGL/Windows_x64_mingw/lib/libglfw3.a"
    LIBS += "../LEti_Engine/OpenGL/Windows_x64_mingw/lib/libglew32.a"

    LIBS += -lopengl32 \
	-luser32 \
	-lgdi32 \
	-lshell32

    INCLUDEPATH += "../LEti_Engine/OpenGL/Windows_x64_mingw/include/"
}

INCLUDEPATH += "../LEti_Engine/OpenGL/GLM"
INCLUDEPATH += "../LEti_Engine/OpenGL/"

INCLUDEPATH += "../Utility/include/"
INCLUDEPATH += "../L_Variable/include/"
INCLUDEPATH += "../LEti_Engine/include/"


SOURCES += \
Main.cpp \

DISTFILES += \
Resources/Font/font_yellow.png \
	Resources/Models/circleish.mdl \
Resources/Models/colliding_object.mdl \
Resources/Models/debug_frame.mdl \
Resources/Models/flat_co.mdl \
Resources/Models/flat_co_2.mdl \
Resources/Models/flat_colliding_object.mdl \
Resources/Models/flat_cos/flat_co_1.mdl \
Resources/Models/flat_cos/flat_co_2.mdl \
Resources/Models/flat_cos/flat_co_3.mdl \
Resources/Models/flat_indicator_red.mdl \
	Resources/Models/hollow_square.mdl \
Resources/Models/intersection_indicator_3d.mdl \
Resources/Models/intersection_point_indicator.mdl \
Resources/Models/pyramid.mdl \
Resources/Models/quad.mdl \
Resources/Models/quad_new.mdl \
Resources/Models/red_cube.mdl \
	Resources/Models/sandclock.mdl \
Resources/Models/text_field.mdl \
Resources/Models/text_field_new.mdl \
Resources/Shaders/fragment_shader.shader \
Resources/Shaders/vertex_shader.shader \
Resources/Textures/quad.png \
Resources/Textures/textures.mdl \
Resources/Textures/ugly_color.png

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../LEti_Engine/OpenGL/GLFW/lib/ -lglfw3
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../LEti_Engine/OpenGL/GLFW/lib/ -lglfw3
