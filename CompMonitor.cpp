#include "CompMonitor.h"
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QSysInfo>
#include <QScreen>


CompMonitor::CompMonitor()
{
    qInfo()<<"      Получение информации о конфигурации системы......\n\n";
    qInfo()<<"      Операционная система:"<<QSysInfo::prettyProductName()<<"\n";
    qInfo()<<"      Сборка:"<<QSysInfo::kernelVersion()<<"\n";

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    qInfo()<<"      CPU architecture:"<<QSysInfo::currentCpuArchitecture()<<"\n";
    qInfo()<<"      Number of processors:"<<systemInfo.dwNumberOfProcessors<<"\n";
    qInfo()<<"      Processor version:"<<systemInfo.wProcessorRevision<<"\n";

    MEMORYSTATUSEX memory_status;
    QStringList system_info;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memory_status)) {

    qInfo()<<"      Total RAM:"<<memory_status.ullTotalPhys / (1024*1024*1024)<<"Gb"<<"\n";
    qInfo()<<"      Free  RAM:"<<memory_status.ullAvailPhys / (1024*1024*1024)<<"Gb"<<"\n\n";

    } else {

        qInfo()<<"   UKNOWN TYPE OF THE MEMORY"<<"\n\n";
    }


    qInfo()<<"_________________________________________________________________________"<<"\n\n";
}
