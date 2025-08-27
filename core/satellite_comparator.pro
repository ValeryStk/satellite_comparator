QT       += core gui xml multimedia multimediawidgets svg testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RC_FILE = resource.rc
CONFIG += c++11

include(bekas/bekas.pri)

SOURCES += \
    cross_square.cpp \
    dynamic_checkbox_widget.cpp \
    google_maps_url_maker.cpp \
    icon_generator.cpp \
    json_utils.cpp \
    layer_list.cpp \
    layer_roi_list.cpp \
    main.cpp \
    main_window_satellite_comparator.cpp \
    MatFilesOperator.cpp \
    message_reporter.cpp \
    progress_informator.cpp \
    qcustomplot.cpp \
    satellite_graphics_view.cpp \
    satellites_structs.cpp \
    sattelite_comparator.cpp \
    sliders_of_image_corrector.cpp \
    text_constants.cpp\
    satellite_xml_reader.cpp \
    udpjsonrpc.cpp

HEADERS += \
    cross_square.h \
    dynamic_checkbox_widget.h \
    google_maps_url_maker.h \
    icon_generator.h \
    json_utils.h \
    layer_list.h \
    layer_roi_list.h \
    main_window_satellite_comparator.h \
    MatFilesOperator.h \
    message_reporter.h \
    progress_informator.h \
    qcustomplot.h \
    satellite_graphics_view.h \
    sattelite_comparator.h \
    sliders_of_image_corrector.h \
    text_constants.h\
    satellite_xml_reader.h \
    satellites_structs.h \
    udpjsonrpc.h \
    version.h

FORMS += \
    main_window_satellite_comparator.ui \
    bekas\GuiModules/SpectrumWidgets/WavesRangeDialog.ui \
    bekas\GuiModules/UasvViewWindow.ui \
    sliders_of_image_corrector.ui

TRANSLATIONS += \
    satellite_comparator_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    res.qrc\

LIBS += -L$$PWD/libs/gdal/x64/lib -lgdal_i
INCLUDEPATH += $$PWD/libs/gdal/x64/include
DEPENDPATH += $$PWD/libs/gdal/x64/include

INCLUDEPATH += $$PWD/libs/matio/src
win32:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/libs/matio/build/Release/ -llibmatio
}
win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/libs/matio/build/Debug/ -llibmatio
}






