QT += core
QT += widgets

DESTDIR = $${OUT_PWD}/..

CONFIG   += c++11
CONFIG   += console
CONFIG   += staticlib
CONFIG   -= app_bundle

TEMPLATE = lib

CONFIG(debug, debug|release){
    TARGET = GitlMVCd
}
CONFIG(release, debug|release){
    TARGET = GitlMVC
}

HEADERS += \
    frontcontroller.h \
    abstractcommand.h \
    view.h \
    updateuievt.h \
    cmdevt.h \
    abstractcommand.h \
    cmdevt.h \
    def.h \
    event.h \
    eventbus.h \
    eventparam.h \
    frontcontroller.h \
    model.h \
    module.h \
    moduledelegate.h \
    updateuievt.h \
    view.h

SOURCES += \
    frontcontroller.cpp \
    view.cpp \
    updateuievt.cpp \
    cmdevt.cpp \
    cmdevt.cpp \
    event.cpp \
    eventbus.cpp \
    eventparam.cpp \
    frontcontroller.cpp \
    module.cpp \
    moduledelegate.cpp \
    updateuievt.cpp \
    view.cpp


