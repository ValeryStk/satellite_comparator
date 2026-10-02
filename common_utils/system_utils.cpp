#include "system_utils.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace su {

void openInNotepad(const QString &text) {
    // 1. Помещаем QString в системный буфер обмена
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
    }

    QTemporaryFile *tempFile = new QTemporaryFile();
    tempFile->setFileTemplate(QDir::tempPath() + "/notepad_temp.txt");

    if (tempFile->open()) {
        tempFile->write(text.toUtf8());
        tempFile->close();
        QProcess::startDetached("notepad.exe", QStringList()
                                                   << tempFile->fileName());
    }
}
}  // namespace su
