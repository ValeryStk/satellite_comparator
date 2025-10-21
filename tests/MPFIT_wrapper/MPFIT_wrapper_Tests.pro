QT += testlib core

include(../../pathes.pri)
include($$MPFIT_DIR/mpfit_wrapper.pri)

TARGET = MPFIT_wrapper_Tests
HEADERS += MPFIT_wrapper_UnitTests.h\

SOURCES += MPFIT_wrapper_UnitTests.cpp\
