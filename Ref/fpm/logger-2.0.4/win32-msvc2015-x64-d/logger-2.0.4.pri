# Add include folder
INCLUDEPATH += $$PWD
export(INCLUDEPATH)

# Add header files
HEADERS += \
    $$PWD/logger.h

export(HEADERS)

# Add source files
SOURCES += \
    $$PWD/logger.cpp

win:LIBS+= -lkernel32

export(SOURCES)

# Make sure we have message context even on release
DEFINES += QT_MESSAGELOGCONTEXT
export(DEFINES)
