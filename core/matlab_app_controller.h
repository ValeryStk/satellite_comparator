#ifndef MATLABAPPCONTROLLER_H
#define MATLABAPPCONTROLLER_H

#include <QObject>
#include <QString>

class MatlabAppController : public QObject {
    Q_OBJECT
public:
    explicit MatlabAppController(QObject *parent = nullptr);

    bool runIfNotRunning();
    bool isRunning();

private:
    bool isProcessRunning(const QString &processName);

private:
    QString exeDir;
    QString fullExePath;

signals:
};

#endif  // MATLABAPPCONTROLLER_H
