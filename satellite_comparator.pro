QT       += core gui xml printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
RC_FILE = resource.rc
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cross_square.cpp \
    dynamic_checkbox_widget.cpp \
    json_utils.cpp \
    main.cpp \
    main_window_satellite_comparator.cpp \
    message_reporter.cpp \
    qcustomplot.cpp \
    satellite_graphics_view.cpp \
    sattelite_comparator.cpp

HEADERS += \
    cross_square.h \
    dynamic_checkbox_widget.h \
    json_utils.h \
    main_window_satellite_comparator.h \
    message_reporter.h \
    qcustomplot.h \
    satellite_graphics_view.h \
    sattelite_comparator.h

FORMS += \
    main_window_satellite_comparator.ui \
    sattelite_comparator.ui

TRANSLATIONS += \
    satellite_comparator_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    res.qrc

LIBS += -L$$PWD/libs/gdal/x64/lib -lgdal_i

INCLUDEPATH += $$PWD/libs/gdal/x64/include
DEPENDPATH += $$PWD/libs/gdal/x64/include


