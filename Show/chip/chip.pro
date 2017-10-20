CONFIG   += c++11 force_debug_info

INCLUDEPATH += $$PWD/../OpenCV/include

TARGET = Chip
TEMPLATE = app
DESTDIR = $$PWD/../build/show

RESOURCES += images.qrc

HEADERS += mainwindow.h \
    view.h \
    chip.h \
    video_widget.h \
    presentation_window.h \
    mainwindowGL.h \
    mygraphicsscene.h \
    presentation_mainwindow.h \
    down_cam.h

SOURCES += main.cpp \
    video_widget.cpp \
    mainwindow.cpp \
    view.cpp \
    chip.cpp \
    presentation_window.cpp \
    mainwindowGL.cpp \
    mygraphicsscene.cpp \
    presentation_mainwindow.cpp \
    down_cam.cpp

QT += widgets
QT += opengl

LIBS += $$PWD/../OpenCV/lib/opencv_world320.lib
win32:LIBS += -lUser32 -lShell32 -lWtsapi32 -lwevtapi -lgdi32

FORMS += \
    mainwindow.ui \
    presentation_mainwindow.ui
