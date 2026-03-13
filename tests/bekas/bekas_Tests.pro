QT += testlib core gui

include(../../pathes.pri)

TARGET = bekas_Tests

INCLUDEPATH += $$BEKAS_DIR $$CORE_DIR

include($$BEKAS_DIR/bekas.pri)
include($$CORE_DIR/matio.pri)
include($$GUI_DIR/spectral_indices_widget.pri)

HEADERS += bekas_UnitTests.h\
           $$CORE_DIR/udpjsonrpc.h\
           $$CORE_DIR/qcustomplot.h\
           $$CORE_DIR/text_constants.h\
           $$CORE_DIR/MatFilesOperator.h\
           $$CORE_DIR/matlab_app_controller.h\

SOURCES += bekas_UnitTests.cpp\
           $$CORE_DIR/udpjsonrpc.cpp\
           $$CORE_DIR/qcustomplot.cpp\
           $$CORE_DIR/text_constants.cpp\
           $$CORE_DIR/MatFilesOperator.cpp\
           $$CORE_DIR/matlab_app_controller.cpp\
