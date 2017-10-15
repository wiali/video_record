QT += core gui
QT += widgets

TARGET = examples
CONFIG   += c++11
CONFIG   -= console
CONFIG   -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS  += \
    testcommand.h \
    testmodel.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui

INCLUDEPATH   += ../../libmvc/src

LIBS += -L$${OUT_PWD}/..

CONFIG(debug, debug|release){
    LIBS += -lGitlMVCd
}
CONFIG(release, debug|release){
    LIBS += -lGitlMVC
}
