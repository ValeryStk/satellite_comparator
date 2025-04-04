#include "message_reporter.h"
#include "QMessageBox"
#include <stddef.h>
#include <QIcon>
#include "QApplication"
#include "QClipboard"


namespace {
void showMessageWithClipboard(const QString& windowTitle,
                 const QString& message,
                 QMessageBox::Icon icon) {
  QMessageBox mb;
  QPushButton *copyButton = mb.addButton("Copy and close", QMessageBox::ActionRole);
  mb.setWindowTitle(windowTitle);
  mb.setInformativeText(message);
  mb.setIcon(icon);
  mb.exec();
  if (mb.clickedButton() == (QAbstractButton*)copyButton) {
      auto clip = QApplication::clipboard();
      clip->setText(message);
  };
};

void showSimpleBox(const QString& windowTitle,
                   const QString& message,
                   QMessageBox::Icon icon){
    QMessageBox mb;
    mb.setWindowTitle(windowTitle);
    mb.setInformativeText(message);
    mb.setIcon(icon);
    mb.exec();
};

}

namespace uts {

void showWarnigMessage(const QString& windowTitle,
                       const QString& message) {
  showSimpleBox(windowTitle,message, QMessageBox::Warning);
};
void showErrorMessage(const QString& windowTitle,
                      const QString& message) {
  showSimpleBox(windowTitle,message, QMessageBox::Critical);
}
void showInfoMessage(const QString &windowTitle,
                     const QString& message) {
  showSimpleBox(windowTitle,message, QMessageBox::Information);
}

void showOkStatus(){
     showSimpleBox("Успеx","Операция успешно выполнена.",QMessageBox::Information);
}
} // end namespace uts
