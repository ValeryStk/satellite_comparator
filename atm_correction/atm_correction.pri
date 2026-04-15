QT+=core gui widgets concurrent
include(../pathes.pri)
include($$MPFIT_DIR/mpfit_wrapper.pri)
include(resources.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$CORE_DIR

SOURCES += \
    $$PWD/atm_correction.cpp \
    $$PWD/satellite_adder.cpp \
    $$PWD/calculation_solver.cpp \
    $$PWD/AtmCorrectionMainWindow.cpp \
    $$CORE_DIR/satellites_structs.cpp \
    $$PWD/bands_widget.cpp \

HEADERS += \
    $$CORE_DIR/common_types.h \
    $$PWD/atm_correction.h \
    $$PWD/common_types.h \
    $$PWD/satellite_adder.h \
    $$PWD/calculation_solver.h \
    $$PWD/AtmCorrectionMainWindow.h \
    $$CORE_DIR/satellite_structs.h \
    $$PWD/bands_widget.h \

FORMS += \
$$PWD/AtmCorrectionMainWindow.ui \

