QT+=core gui widgets concurrent printsupport
TEMPLATE=app
include(..\pathes.pri)
include(atm_correction.pri)

INCLUDEPATH+=$$CORE_DIR \
$$DAVIS_DIR \

HEADERS += \
           $$CORE_DIR/json_utils.h \
           $$DAVIS_DIR/davis.h \

SOURCES += \
           $$CORE_DIR/json_utils.cpp \
           $$DAVIS_DIR/davis.cpp \
           main.cpp\

FORMS += \
    AtmCorrectionMainWindow.ui \
