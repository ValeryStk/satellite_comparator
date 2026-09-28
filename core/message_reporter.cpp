#include "message_reporter.h"

#include <stddef.h>

#include <QIcon>

#include "QApplication"
#include "QClipboard"
#include "QMessageBox"

namespace {
void showMessageWithClipboard(const QString& windowTitle,
                              const QString& message, QMessageBox::Icon icon) {
    QMessageBox mb;
    QPushButton* copyButton =
        mb.addButton("Copy and close", QMessageBox::ActionRole);
    mb.setWindowTitle(windowTitle);
    mb.setInformativeText(message);
    mb.setIcon(icon);
    mb.exec();
    if (mb.clickedButton() == (QAbstractButton*)copyButton) {
        auto clip = QApplication::clipboard();
        clip->setText(message);
    };
};

void showSimpleBox(const QString& windowTitle, const QString& message,
                   QMessageBox::Icon icon) {
    QMessageBox mb;
    mb.setWindowTitle(windowTitle);
    mb.setInformativeText(message);
    mb.setIcon(icon);
    mb.exec();
};

}  // namespace

namespace uts {

void showWarnigMessage(const QString& windowTitle, const QString& message) {
    showSimpleBox(windowTitle, message, QMessageBox::Warning);
};
void showErrorMessage(const QString& windowTitle, const QString& message) {
    showSimpleBox(windowTitle, message, QMessageBox::Critical);
}
void showInfoMessage(const QString& windowTitle, const QString& message) {
    showSimpleBox(windowTitle, message, QMessageBox::Information);
}

void showOkStatus() {
    showSimpleBox("Успеx", "Операция успешно выполнена.",
                  QMessageBox::Information);
}

void showOperationFailed() {
    showSimpleBox("Не удалось выполнить операцию",
                  "Пожалуйста, проверьте правильность введенных данных и "
                  "повторите попытку. "
                  "Если ошибка повторится, обратитесь в техническую поддержку.",
                  QMessageBox::Warning);
}

void showNoDataAvailable() {
    uts::showWarnigMessage(
        "Изображение не доступно",
        "Пожалуйста, проверьте загрузку данных. "
        "Если ошибка повторится, обратитесь в техническую поддержку.");
}

}  // end namespace uts
