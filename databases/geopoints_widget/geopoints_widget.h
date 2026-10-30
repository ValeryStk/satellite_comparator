#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

struct PointData {
    int pointId;
    QString pointName;
    QString productUri;
    double latitude;
    double longitude;
};

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    bool insertGeoPoint(const QString &productUri, double latitude,
                        double longitude, const QString &name,
                        const QString &description = QString());
    bool linkPointToUri(int pointId, const QString &productUri);

    QVector<PointData> selectPointsByProduct(const QString &productUri);

private slots:
    void on_pushButton_clicked();
    void showContextMenu(const QPoint &pos);

    void on_pushButton_insert_data_clicked();

    void on_pushButton_clear_all_data_clicked();

private:
    Ui::Widget *ui;
    bool initDataBase(const QString &dbPath);
    void generateAndInsertRandomPoints(int count);
    bool clearAllData();
};
#endif  // WIDGET_H
