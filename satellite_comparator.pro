QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    json_utils.cpp \
    main.cpp \
    main_window_satellite_comparator.cpp \
    message_reporter.cpp \
    qcustomplot.cpp \
    sattelite_comparator.cpp

HEADERS += \
    json_utils.h \
    main_window_satellite_comparator.h \
    message_reporter.h \
    qcustomplot.h \
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

DISTFILES +=

RESOURCES += \
    res.qrc

LIBS += libs/gdal/x64 -lgdal_i
INCLUDEPATH += libs/gdal/x64/include
DEPENDPATH += libs/gdal/x64/include
