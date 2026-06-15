QT+=core gui widgets concurrent

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/calculation_solver.cpp \
    $$PWD/LeastSquareSolver.cpp \
    $$PWD/mpfit.c \
    $$PWD/CompMonitor.cpp \
    $$PWD/QrcFilesRestorer.cpp \
    $$PWD/DBJson.cpp \


HEADERS += \
    $$PWD/calculation_solver.h \
    $$PWD/LeastSquareSolver.h \
    $$PWD/mpfit.h \
    $$PWD/CompMonitor.h \
    $$PWD/common_types.h \
    $$PWD/QrcFilesRestorer.h \
    $$PWD/DBJson.h \


