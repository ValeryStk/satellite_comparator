#include "system_utils.h"

#include <QApplication>
#include <QClipboard>
#include <QProcess>
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

    // 2. Запускаем стандартный Блокнот Windows (notepad.exe)
    QProcess::startDetached("notepad.exe");

    // 3. Пауза, чтобы Блокнот успел открыться и получить фокус
    QThread::msleep(500);

    // 4. Эмулируем нажатие Ctrl+V для вставки текста из буфера
#ifdef Q_OS_WIN
    // Нажимаем Ctrl
    keybd_event(VK_CONTROL, 0, 0, 0);
    // Нажимаем V
    keybd_event('V', 0, 0, 0);
    // Отпускаем V
    keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
    // Отпускаем Ctrl
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
#endif
}

}  // namespace su
