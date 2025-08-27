INCLUDEPATH += $$PWD/libs/matio/src
win32:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/libs/matio/build/Release/ -llibmatio
}
win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/libs/matio/build/Debug/ -llibmatio
}