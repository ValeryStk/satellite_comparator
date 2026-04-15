#ifndef BANDS_WIDGET_H
#define BANDS_WIDGET_H

#include <QCheckBox>
#include <QList>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

class BandsWidget : public QWidget {
    Q_OBJECT
public:
    explicit BandsWidget(const QList<QString>& labels,
                         QVBoxLayout* layout = nullptr);

    QVector<QPair<int, int>> get_choosed_bands();

    void setInitialCheckBoxesToggled(const QVector<int>& toToggle);

    void setRGBchannels();

    void clear();

private slots:
    void onCheckboxStateChanged(QCheckBox* checkBox);

signals:
    void choosed_bands_changed();

private:
    QVBoxLayout* m_layout;
    QList<QCheckBox*> m_checkboxes;
    QList<QPair<QCheckBox*, int>>
        m_checkedOrder;  // Список для отслеживания порядка выбора чекбоксов
};

#endif  // BANDS_WIDGET_H
