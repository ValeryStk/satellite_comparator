QT += widgets

include(../pathes.pri)
INCLUDEPATH += $$PWD
INCLUDEPATH += $$SAM_DIR


HEADERS += \
          $$PWD/spectral_indices_widget.h\
          $$SAM_DIR/health_ranges.h\

SOURCES += \
          $$PWD/spectral_indices_widget.cpp\
          $$SAM_DIR/health_ranges.cpp\
