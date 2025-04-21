#ifndef DYNAMIC_CHECKBOX_WIDGET_H
#define DYNAMIC_CHECKBOX_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QString>
#include <QList>

class DynamicCheckboxWidget : public QWidget {
    Q_OBJECT
public:
    explicit DynamicCheckboxWidget(const QList<QString>& labels,
                                   QWidget *parent = nullptr,
                                   QVBoxLayout *layout = nullptr);

QVector<QPair<QString,int>> get_choosed_bands();

private slots:
    void onCheckboxStateChanged(QCheckBox *checkBox);

signals:
    void choosed_bands_changed();

private:
    QList<QCheckBox*> checkboxes;
    QList<QPair<QCheckBox*,int>> checkedOrder; // Список для отслеживания порядка выбора чекбоксов
    const int maxChecked = 3;
    bool used_colors[3] = {false,false,false};//RGB
};

#endif // DYNAMIC_CHECKBOX_WIDGET_H
