TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += sliders_of_image_corrector 
sliders_of_image_corrector.file = sliders_of_image_corrector/sliders_of_image_corrector_Tests.pro

SUBDIRS += satellite_comparator
satellite_comparator.file = satellite_comparator/satellite_comparator_Tests.pro

SUBDIRS += sam
sam.file = sam/sam_Tests.pro
