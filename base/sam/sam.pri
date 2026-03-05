SAM_DIR =$$PWD
INCLUDEPATH += $$SAM_DIR

SOURCES += \
    $$PWD/satellites_bands_map.cpp \
    $$SAM_DIR/sam.cpp \
    $$SAM_DIR/health_ranges.cpp \



HEADERS += \
    $$PWD/satellites_bands_map.h \
    $$SAM_DIR/sam.h \
    $$SAM_DIR/health_ranges.h \

