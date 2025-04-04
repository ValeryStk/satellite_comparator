#ifndef MESSAGE_REPORTER_H
#define MESSAGE_REPORTER_H

class QString;

namespace uts {

void showWarnigMessage(const QString& windowTitle,
                       const QString& message);
void showErrorMessage(const QString& windowTitle,
                      const QString& message);
void showInfoMessage(const QString& windowTitle,
                     const QString& message);
void showOkStatus();

} // end namespace uts

#endif // MESSAGE_REPORTER_H
