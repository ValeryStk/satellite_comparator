QT += widgets printsupport core network

include(../../pathes.pri)

INCLUDEPATH += $$BEKAS_DIR $$CORE_DIR

SOURCES += \
    $$BEKAS_DIR/BaseTools/DBJson.cpp\
    $$BEKAS_DIR/BaseTools/IniFileLoader.cpp\
    $$BEKAS_DIR/BaseTools/QrcFilesRestorer.cpp\
    $$BEKAS_DIR/GuiModules/ImageWidgets/PhotospectralGraphicsView.cpp\
    $$BEKAS_DIR/GuiModules/SpectrumWidgets/WavesRangeDialog.cpp\
    $$BEKAS_DIR/GuiModules/SpectrumWidgets/SpectrPlotterWidget.cpp\
    $$BEKAS_DIR/GuiModules/UasvViewWindow.cpp \
    $$BEKAS_DIR/ProcessingModules/FilesParser.cpp\
    $$BEKAS_DIR/ProcessingModules/SpectrDataSaver.cpp\


HEADERS += \
    $$BEKAS_DIR/BaseTools/DBJson.h\
    $$BEKAS_DIR/BaseTools/IniFileLoader.h\
    $$BEKAS_DIR/BaseTools/QrcFilesRestorer.h\
    $$BEKAS_DIR/GuiModules/ImageWidgets/PhotospectralGraphicsView.h\
    $$BEKAS_DIR/GuiModules/SpectrumWidgets/WavesRangeDialog.h\
    $$BEKAS_DIR/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h\
    $$BEKAS_DIR/GuiModules/UasvViewWindow.h\
    $$BEKAS_DIR/ProcessingModules/SpectrDataSaver.h\
    $$BEKAS_DIR/ProcessingModules/FilesParser.h\

FORMS += \
    $$BEKAS_DIR/GuiModules/SpectrumWidgets/WavesRangeDialog.ui\
    $$BEKAS_DIR/GuiModules/UasvViewWindow.ui\
