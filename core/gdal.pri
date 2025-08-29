GDAL_DIR = $$PWD/libs/gdal/x64

LIBS += -L$$GDAL_DIR/lib -lgdal_i
INCLUDEPATH += $$GDAL_DIR/include
DEPENDPATH  += $$GDAL_DIR/include