QT += testlib core widgets
include(../../pathes.pri)

INCLUDEPATH += $$CORE_DIR

TARGET = message_reporter_Tests

HEADERS += message_reporter_UnitTests.h\
           $$CORE_DIR/message_reporter.h\

SOURCES += message_reporter_UnitTests.cpp\
           $$CORE_DIR/message_reporter.cpp\
