MATIO_DIR = $$PWD/libs/matio

INCLUDEPATH += $$MATIO_DIR/src
win32:CONFIG(release, debug|release) {
    LIBS += -L$$MATIO_DIR/build/Release/ -llibmatio
}
win32:CONFIG(debug, debug|release) {
    LIBS += -L$$MATIO_DIR/build/Debug/ -llibmatio
}
