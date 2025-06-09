QT       += core gui xml multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RC_FILE = resource.rc
CONFIG += c++11


SOURCES += \
    cross_square.cpp \
    dynamic_checkbox_widget.cpp \
    google_maps_url_maker.cpp \
    json_utils.cpp \
    main.cpp \
    main_window_satellite_comparator.cpp \
    message_reporter.cpp \
    progress_informator.cpp \
    qcustomplot.cpp \
    satellite_graphics_view.cpp \
    sattelite_comparator.cpp \
    text_constants.cpp\
    bekas/BaseTools/DBJson.cpp \
    bekas/BaseTools/IniFileLoader.cpp \
    bekas/BaseTools/QrcFilesRestorer.cpp \
    bekas/GuiModules/ImageWidgets/PhotospectralGraphicsView.cpp \
    bekas/GuiModules/SpectrumWidgets/WavesRangeDialog.cpp \
    bekas/ProcessingModules/FilesParser.cpp \
    bekas/GuiModules/SpectrumWidgets/SpectrPlotterWidget.cpp \
    bekas/ProcessingModules/SpectrDataSaver.cpp\
    bekas/GuiModules/UasvViewWindow.cpp

HEADERS += \
    cross_square.h \
    dynamic_checkbox_widget.h \
    google_maps_url_maker.h \
    json_utils.h \
    main_window_satellite_comparator.h \
    message_reporter.h \
    progress_informator.h \
    qcustomplot.h \
    satellite_graphics_view.h \
    sattelite_comparator.h \
    text_constants.h\
    bekas/BaseTools/DBJson.h \
    bekas/BaseTools/IniFileLoader.h \
    bekas/BaseTools/QrcFilesRestorer.h \
    bekas/GuiModules/ImageWidgets/PhotospectralGraphicsView.h \
    bekas/GuiModules/SpectrumWidgets/WavesRangeDialog.h \
    bekas/ProcessingModules/FilesParser.h \
    bekas/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h \
    bekas/ProcessingModules/SpectrDataSaver.h \
    bekas/GuiModules/UasvViewWindow.h

FORMS += \
    main_window_satellite_comparator.ui \
    sattelite_comparator.ui\
    bekas\GuiModules/SpectrumWidgets/WavesRangeDialog.ui \
    bekas\GuiModules/UasvViewWindow.ui

TRANSLATIONS += \
    satellite_comparator_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    res.qrc\
    bekas\res.qrc

LIBS += -L$$PWD/libs/gdal/x64/lib -lgdal_i

INCLUDEPATH += $$PWD/libs/gdal/x64/include
DEPENDPATH += $$PWD/libs/gdal/x64/include


