INCLUDEPATH += $$PWD/../OpenCV/include

TARGET = Chip
TEMPLATE = app
DESTDIR = $$PWD/../build/show

RESOURCES += images.qrc

HEADERS += mainwindow.h \
    view.h \
    chip.h \
    video_widget.h \
    presentation_window.h

SOURCES += main.cpp \
    video_widget.cpp \
    mainwindow.cpp \
    view.cpp \
    chip.cpp \
    presentation_window.cpp

QT += widgets
QT += opengl
