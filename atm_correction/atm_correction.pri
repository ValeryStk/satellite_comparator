include(../pathes.pri)
include($$MPFIT_DIR/mpfit_wrapper.pri)
include(resources.pri)

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/atm_correction.cpp \
    $$PWD/satellite_adder.cpp \
    $$PWD/calculation_solver.cpp \

HEADERS += \
    $$PWD/atm_correction.h \
    $$PWD/common_types.h \
    $$PWD/satellite_adder.h \
    $$PWD/calculation_solver.h \

