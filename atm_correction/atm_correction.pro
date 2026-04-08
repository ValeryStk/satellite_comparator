QT+=core gui widgets
TEMPLATE=app
include(..\pathes.pri)
include(atm_correction.pri)

INCLUDEPATH+=$$CORE_DIR

HEADERS += \
           $$CORE_DIR/json_utils.h \
           AtmCorrectionMainWindow.h \

SOURCES += \
           $$CORE_DIR/json_utils.cpp \
    AtmCorrectionMainWindow.cpp \
           main.cpp\

FORMS += \
    AtmCorrectionMainWindow.ui \
