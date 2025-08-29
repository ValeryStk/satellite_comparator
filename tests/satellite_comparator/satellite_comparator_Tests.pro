QT += testlib core widgets printsupport

CORE_DIR  = $$PWD/../../core
DAVIS_DIR = $$PWD/../../third_party/davis

INCLUDEPATH = $$CORE_DIR $$DAVIS_DIR

TARGET = SatelliteComparatorTests

include($$CORE_DIR/resources.pri)
include($$DAVIS_DIR/davis.pri)

HEADERS +=\
           $$CORE_DIR/sattelite_comparator.h\
           $$CORE_DIR/json_utils.h \
           SatelliteComparatorUnitTests.h

SOURCES +=\
           $$CORE_DIR/sattelite_comparator.cpp\
           $$CORE_DIR/json_utils.cpp \
           SatelliteComparatorUnitTests.cpp



