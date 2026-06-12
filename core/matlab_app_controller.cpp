#include "matlab_app_controller.h"

#include <windows.h>

#include <QApplication>
#include <QMessageBox>
#include <QProcess>
//
#include <tlhelp32.h>  // обязатолено после #include <windows.h>

#include "MatFilesOperator.h"

const bool IS_NEED_CHECK_RUNNING_EXE = true;
const QString NETWORK_CONFIG_FILE_NAME = "network_config.ini";

MatlabAppController::MatlabAppController(QObject *parent) : QObject(parent) {
    exeDir = QCoreApplication::applicationDirPath();
    fullExePath =
        exeDir + "/" + matlabAppDirRelativeName + "/" + matlabAppExeFile;
}

bool MatlabAppController::runIfNotRunning() {
    if (!isRunning()) {
        QString configPath = exeDir + "/" + NETWORK_CONFIG_FILE_NAME;
        // Передаём через переменную окружения
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("FOREST_GUARD_CONFIG", configPath);

        QProcess *process = new QProcess(this);
        process->setProcessEnvironment(env);
        process->startDetached(fullExePath, {});
        return true;

    } else {
        QMessageBox::information(nullptr, "Info",
                                 "Spectra classifier is already running");
        return false;
    }
}

bool MatlabAppController::isRunning() {
    if (IS_NEED_CHECK_RUNNING_EXE)
        return isProcessRunning(matlabAppExeFile);
    else
        return true;
}

bool MatlabAppController::isProcessRunning(const QString &processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            QString exe = QString::fromWCharArray(pe.szExeFile);
            if (exe.compare(processName, Qt::CaseInsensitive) == 0) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return false;
}
