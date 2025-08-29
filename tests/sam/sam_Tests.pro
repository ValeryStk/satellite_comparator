QT += testlib core

include(../../pathes.pri)

INCLUDEPATH = $$SAM_DIR

TARGET = samTests

HEADERS +=\
    $$SAM_DIR/sam.h\
    SamUnitTests.h


SOURCES +=\
    $$SAM_DIR/sam.cpp\
    SamUnitTests.cpp
