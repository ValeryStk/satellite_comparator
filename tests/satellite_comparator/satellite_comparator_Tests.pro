QT += testlib core widgets printsupport

TARGET = SatelliteComparatorTests

HEADERS +=\
           ../../core/sattelite_comparator.h\
           ../../core/json_utils.h \
           SatelliteComparatorUnitTests.h

SOURCES +=\
           ../../core/sattelite_comparator.cpp\
           ../../core/json_utils.cpp \
           SatelliteComparatorUnitTests.cpp



