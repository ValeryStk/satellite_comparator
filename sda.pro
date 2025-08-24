TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += base core tests

base.file  = base/base.pro
core.file  = core/satellite_comparator.pro
tests.file = tests/tests.pro

