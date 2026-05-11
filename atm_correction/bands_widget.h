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

    QVector<int> get_choosed_bands();

    void clear();
    void updateCheckboxesList(const QList<QString>& labels);

private slots:
    void onCheckboxStateChanged(QCheckBox* checkBox);

signals:
    void choosed_bands_changed();

private:
    QVBoxLayout* m_layout;
    QList<QCheckBox*> m_checkboxes;
};

#endif  // BANDS_WIDGET_H
