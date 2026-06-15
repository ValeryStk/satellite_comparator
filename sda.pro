#sda Satellite Data Analyzer
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += base core tests atm_correction cat_atm_correction

base.file  = base/base.pro
core.file  = core/satellite_comparator.pro
tests.file = tests/tests.pro

