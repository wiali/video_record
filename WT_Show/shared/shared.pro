include($$PWD/fpm.pri)

DEFINES += SHARED_LIBRARY
include($$PWD/shared.pri)

QT += widgets network
CONFIG += force_debug_info

TARGET = shared
TEMPLATE = lib

DESTDIR = $$PWD/../build/$$FORTIS_SPEC/shared

PRECOMPILED_HEADER = stable.h

win32:LIBS += -lDbghelp -lAdvapi32 -luser32

unix {
    target.path = /usr/lib
    INSTALLS += target
}

# copy all headers
QMAKE_POST_LINK += $$quote(cmd /c xcopy /Y /I $$system_quote($$shell_path($$PWD/*.h)) $$system_quote($$shell_path($$DESTDIR/include/)) $$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(cmd /c xcopy /Y /I $$system_quote($$shell_path($$PWD/utils/*.h)) $$system_quote($$shell_path($$DESTDIR/include/)) $$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(cmd /c xcopy /Y /I $$system_quote($$shell_path($$PWD/Ink/ColorPicker/*.h)) $$system_quote($$shell_path($$DESTDIR/include/)) $$escape_expand(\\n\\t))

# Deploy on both camera and stage
# Thisis indeed duplicated, as the Camera and Stage projhects have their own copy events, but, as just clicking build was not forcing the copy
# I am leaving this here for now
QMAKE_POST_LINK += $$quote(cmd /c xcopy /S /Y /I $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/shared/*.dll)) $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/capture)) $$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(cmd /c xcopy /S /Y /I $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/shared/*.dll)) $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/stage)) $$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(cmd /c xcopy /S /Y /I $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/shared/*.dll)) $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/assisted_segmentation)) $$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(cmd /c xcopy /S /Y /I $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/shared/*.dll)) $$system_quote($$shell_path($$PWD/../build/$$FORTIS_SPEC/edit_functionality)) $$escape_expand(\\n\\t))

DISTFILES += \
    Resources/shaders/brightnessContrast.frag \
    Resources/shaders/vertexshader.vert \
    fpm.pri
