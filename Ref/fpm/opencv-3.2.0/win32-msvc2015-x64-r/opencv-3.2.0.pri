# Add include folder
INCLUDEPATH += $$PWD/include
export(INCLUDEPATH)

# Add libraries path
QMAKE_LIBDIR += $$PWD/lib
export(QMAKE_LIBDIR)

# Add libraries
LIBS += -l$$qtLibraryTarget(opencv_world320)
export(LIBS)
