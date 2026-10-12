QT += testlib core widgets printsupport

include(../../pathes.pri)

INCLUDEPATH = $$CORE_DIR $$DAVIS_DIR $$ATM_CORR_DIR

TARGET = SatelliteComparatorTests

include($$CORE_DIR/resources.pri)
include($$DAVIS_DIR/davis.pri)
include($$ATM_CORR_DIR/resources.pri)

HEADERS +=\
           $$CORE_DIR/sattelite_comparator.h\
           $$CORE_DIR/json_utils.h \
           SatelliteComparatorUnitTests.h

SOURCES +=\
           $$CORE_DIR/sattelite_comparator.cpp\
           $$CORE_DIR/json_utils.cpp \
           SatelliteComparatorUnitTests.cpp



