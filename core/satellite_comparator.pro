QT       += core gui xml multimedia multimediawidgets svg testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RC_FILE = resource.rc
CONFIG += c++11

include(gdal.pri)
include(bekas/bekas.pri)
include(matio.pri)
include(resources.pri)
include(../translations/translations.pri)

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
    sliders_of_image_corrector.ui

