QT += widgets printsupport core network

BEKAS_ROOT = $$PWD
INCLUDEPATH += BEKAS_ROOT $$PWD/../

SOURCES += \
    $$BEKAS_ROOT/BaseTools/DBJson.cpp \
    $$BEKAS_ROOT/BaseTools/IniFileLoader.cpp \
    $$BEKAS_ROOT/BaseTools/QrcFilesRestorer.cpp \
    $$BEKAS_ROOT/GuiModules/ImageWidgets/PhotospectralGraphicsView.cpp \
    $$BEKAS_ROOT/GuiModules/SpectrumWidgets/WavesRangeDialog.cpp \
    $$BEKAS_ROOT/ProcessingModules/FilesParser.cpp \
    $$BEKAS_ROOT/GuiModules/SpectrumWidgets/SpectrPlotterWidget.cpp \
    $$BEKAS_ROOT/ProcessingModules/SpectrDataSaver.cpp\
    $$BEKAS_ROOT/GuiModules/UasvViewWindow.cpp \

HEADERS += \
    $$BEKAS_ROOT/BaseTools/DBJson.h \
    $$BEKAS_ROOT/BaseTools/IniFileLoader.h \
    $$BEKAS_ROOT/BaseTools/QrcFilesRestorer.h \
    $$BEKAS_ROOT/GuiModules/ImageWidgets/PhotospectralGraphicsView.h \
    $$BEKAS_ROOT/GuiModules/SpectrumWidgets/WavesRangeDialog.h \
    $$BEKAS_ROOT/ProcessingModules/FilesParser.h \
    $$BEKAS_ROOT/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h \
    $$BEKAS_ROOT/ProcessingModules/SpectrDataSaver.h \
    $$BEKAS_ROOT/GuiModules/UasvViewWindow.h \

FORMS += \
    $$BEKAS_ROOT/GuiModules/SpectrumWidgets/WavesRangeDialog.ui \
    $$BEKAS_ROOT/GuiModules/UasvViewWindow.ui \
