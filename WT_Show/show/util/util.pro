QT += core testlib
QT  -= gui

TARGET = TestMVC
CONFIG   += console
CONFIG   += c++11
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH  += ../../libmvc/src

SOURCES += \
    testcase.cpp \
    #testcase2.cpp \
    #testmacro.cpp \

LIBS += -L$${OUT_PWD}/..

CONFIG(debug, debug|release){
    LIBS += -lGitlMVCd
}
CONFIG(release, debug|release){
    LIBS += -lGitlMVC
}
