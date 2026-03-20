QT += testlib core

include(../../pathes.pri)
include ($$ATM_CORR_DIR/atm_correction.pri)

INCLUDEPATH += $$CORE_DIR

TARGET = atm_correction_Tests

HEADERS += atm_correction_UnitTests.h\
           $$CORE_DIR/json_utils.h \

SOURCES += atm_correction_UnitTests.cpp\
           $$CORE_DIR/json_utils.cpp \
