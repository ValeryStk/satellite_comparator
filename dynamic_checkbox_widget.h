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
    explicit DynamicCheckboxWidget(const QList<QString>& labels, QWidget *parent = nullptr);

private slots:
    void onCheckboxStateChanged(QCheckBox *checkBox,
                                int state);

private:
    QList<QCheckBox*> checkboxes;
    QList<QCheckBox*> checkedOrder; // Список для отслеживания порядка выбора чекбоксов
    const int maxChecked = 3;
};

#endif // DYNAMIC_CHECKBOX_WIDGET_H
