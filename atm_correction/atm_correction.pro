QT+=core gui widgets concurrent printsupport
TEMPLATE=app
include(..\pathes.pri)
include(atm_correction.pri)

INCLUDEPATH+=$$CORE_DIR

HEADERS += \
           $$CORE_DIR/json_utils.h \

SOURCES += \
           $$CORE_DIR/json_utils.cpp \
           main.cpp\

FORMS += \
    AtmCorrectionMainWindow.ui \
