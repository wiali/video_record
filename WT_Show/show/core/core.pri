include($$PWD/../fpm.pri)
include($$PWD/../../shared/shared.pri)

QT       += core widgets gui multimedia multimediawidgets concurrent svg printsupport
CONFIG   += c++11 force_debug_info

win32:LIBS += -lUser32 -lShell32 -lWtsapi32 -lwevtapi

RC_ICONS += $$PWD/Resources/production/show.ico

PRECOMPILED_HEADER = $$PWD/stable.h
INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/show_worktool.cpp \
    $$PWD/common/*.cpp \
    $$PWD/components/*.cpp \
    $$PWD/event/*.cpp \
    $$PWD/mat/*.cpp \
    $$PWD/model/*.cpp \
    $$PWD/monitor/*.cpp \
    $$PWD/presentation/*.cpp

HEADERS += \
    $$PWD/stable.h \
    $$PWD/common/*.h \
    $$PWD/components/*.h \
    $$PWD/event/*.h \
    $$PWD/mat/*.h \
    $$PWD/model/*.h \
    $$PWD/monitor/*.h \
    $$PWD/presentation/*.h

FORMS += \
    $$PWD/mat/*.ui \
    $$PWD/monitor/*.ui \
    $$PWD/presentation/*.ui

RESOURCES += \
    $$PWD/show.qrc

OTHER_FILES += \
    $$PWD/../package.json \
    $$PWD/../README.md \
    $$PWD/CHANGELOG.md

#this code is required due to QTBUG-48416
#the bug states that lrelease doesn't run on subdirs
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all

win32:QMAKE_LFLAGS += /INCREMENTAL /debug:fastlink