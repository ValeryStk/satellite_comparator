#ifndef COMPMONITOR
#define COMPMONITOR_H
#include <Windows.h>
#include <QObject>


class CompMonitor:public QObject

{
    Q_OBJECT
public:
    CompMonitor();
};

#endif // COMPMONITOR_H
