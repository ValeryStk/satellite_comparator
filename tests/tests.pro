QT += testlib core widgets printsupport
TARGET = UnitTests

HEADERS += UnitTests.h\
           ../core/sliders_of_image_corrector.h\
           ../core/sattelite_comparator.h

SOURCES += UnitTests.cpp\
           ../core/sliders_of_image_corrector.cpp\
           ../core/sattelite_comparator.cpp

FORMS += ../core/sliders_of_image_corrector.ui





