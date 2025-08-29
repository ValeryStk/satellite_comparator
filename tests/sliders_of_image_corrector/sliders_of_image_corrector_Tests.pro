QT += testlib core widgets printsupport

include(../../pathes.pri)

TARGET = SlidersOfImageCorrectorTest

HEADERS +=\
         $$CORE_DIR/sliders_of_image_corrector.h\
         SlidersOfImageCorrectorUnitTests.h

SOURCES +=\
         $$CORE_DIR/sliders_of_image_corrector.cpp\
         SlidersOfImageCorrectorUnitTests.cpp

FORMS += $$CORE_DIR/sliders_of_image_corrector.ui





