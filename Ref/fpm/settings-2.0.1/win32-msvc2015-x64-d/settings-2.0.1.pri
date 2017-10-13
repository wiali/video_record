# Add include folder
INCLUDEPATH += $$PWD
export(INCLUDEPATH)

# Add header files
HEADERS += \
    $$PWD/settings.h

export(HEADERS)

# Add source files
SOURCES += \
    $$PWD/settings.cpp

export(SOURCES)
