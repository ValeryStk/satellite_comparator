#ifndef ATMCORRECTIONMAINWINDOW_H
#define ATMCORRECTIONMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class AtmCorrectionMainWindow;
}

class AtmCorrectionMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AtmCorrectionMainWindow(QWidget *parent = nullptr);
    ~AtmCorrectionMainWindow();

private:
    Ui::AtmCorrectionMainWindow *ui;
};

#endif // ATMCORRECTIONMAINWINDOW_H
