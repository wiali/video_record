#-------------------------------------------------
#
# Project created by QtCreator 2017-10-09T04:31:40
#
#-------------------------------------------------

QT += core gui

QT += widgets

TARGET = QT_record
TEMPLATE = app

INCLUDEPATH += $$PWD\..\video_record\libav\include

SOURCES += main.cpp\
        mainwindow.cpp \
    audio_recording.cpp \
    desktop_record.cpp \
    video_recording.cpp

HEADERS  += mainwindow.h \
    audio_recording.h \
    desktop_record.h \
    video_recording.h

LIBDIR = $$PWD\..\video_record\libav\libs
#LIBS += -avcodec -avdevice -avfilter -avformat -avresample -avutil -swscale

FORMS    += mainwindow.ui
