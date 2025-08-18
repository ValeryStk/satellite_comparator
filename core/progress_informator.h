#ifndef PROGRESSINFORMATOR_H
#define PROGRESSINFORMATOR_H

#include <QLabel>

class ProgressInformator : public QLabel
{
public:
    ProgressInformator(QWidget* parent,
                       const QString &text);
};

#endif // PROGRESSINFORMATOR_H
