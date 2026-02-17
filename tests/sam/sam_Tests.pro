QT += testlib core

include(../../pathes.pri)

INCLUDEPATH = $$SAM_DIR

TARGET = samTests

HEADERS +=\
    $$SAM_DIR/sam.h\
    $$SAM_DIR/satellites_bands_map.h \
    SamUnitTests.h


SOURCES +=\
    $$SAM_DIR/sam.cpp\
    $$SAM_DIR/satellites_bands_map.cpp \
    SamUnitTests.cpp
