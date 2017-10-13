#DEFINES += SHARED_LIBRARY

#PRECOMPILED_HEADER += $$PWD/stable.h
#export(PRECOMPILED_HEADER)

QT += xml

INCLUDEPATH += $$PWD $$PWD/utils
export(INCLUDEPATH)

SOURCES += \
    $$PWD/single_instance.cpp \
    $$PWD/frame_counting.cpp \
    $$PWD/global_utilities.cpp

HEADERS += \
    $$PWD/shared_global.h \
    $$PWD/single_instance.h \
    $$PWD/frame_counting.h \
    $$PWD/global_utilities.h

RESOURCES += \
    $$PWD/shared.qrc \

export(RESOURCES)

win32:SOURCES += $$PWD/crash_handler.cpp
win32:HEADERS += $$PWD/crash_handler.h
win32:LIBS += -lDbghelp -lAdvapi32 -lUser32 -lShcore

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \

DISTFILES += \
    $$PWD/Resources/shaders/brightnessContrast.frag \
    $$PWD/Resources/shaders/vertexshader.vert
