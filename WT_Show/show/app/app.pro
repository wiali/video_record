include($$PWD/../core/core.pri)

TARGET = Show
TEMPLATE = app
DESTDIR = $$PWD/../../build/$$FORTIS_SPEC/Show

SOURCES += \
    main.cpp

WINDEPLOYQT_ARGS += -multimediawidgets -websockets --printsupport --no-translations
deployDependencies()
fillTargetMetaData($$PWD/../package.json)
extractMetaData($$PWD/../package.json)
