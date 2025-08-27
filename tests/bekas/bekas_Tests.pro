QT += testlib core gui
TARGET = bekas_Tests

INCLUDEPATH += $$PWD/../../core/bekas $$PWD/../../core/

include(../../core/bekas/bekas.pri)
include(../../core/matio.pri)

HEADERS += bekas_UnitTests.h\
           ../../core/udpjsonrpc.h\
           ../../core/qcustomplot.h\
           ../../core/text_constants.h\
           ../../core/MatFilesOperator.h\

SOURCES += bekas_UnitTests.cpp\
           ../../core/udpjsonrpc.cpp\
           ../../core/qcustomplot.cpp\
           ../../core/text_constants.cpp\
           ../../core/MatFilesOperator.cpp\
