QT += widgets

IMAGE_VIEWER_DIR = $$PWD

INCLUDEPATH += $$IMAGE_VIEWER_DIR

HEADERS += \
          $$IMAGE_VIEWER_DIR/image_viewer.h\
          $$IMAGE_VIEWER_DIR/view_sync_manager.h\

SOURCES += \
          $$IMAGE_VIEWER_DIR/image_viewer.cpp\
          $$IMAGE_VIEWER_DIR/view_sync_manager.cpp\