#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    bool insertGeoPoint(const QString &productUri, double latitude,
                        double longitude, const QString &name,
                        const QString &description = QString());

private slots:
    void on_pushButton_clicked();
    void showContextMenu(const QPoint &pos);

private:
    Ui::Widget *ui;
    bool initDataBase(const QString &dbPath);
    void generateAndInsertRandomPoints(int count);
};
#endif  // WIDGET_H
