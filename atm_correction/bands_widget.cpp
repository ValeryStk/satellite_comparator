#include "bands_widget.h"

#include <algorithm>

#include "QDebug"

namespace {
bool isRangeValid(const QVector<int>& vec, const int low, const int max) {
    return std::all_of(vec.begin(), vec.end(), [low, max](int value) {
        return value >= low && value <= max;
    });
}
void clearLayout(QLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();  // Удаляем виджет
        }
        delete item;  // Удаляем сам элемент
    }
}
}  // namespace

BandsWidget::BandsWidget(const QList<QString>& labels, QVBoxLayout* layout) {
    m_layout = layout;
    updateCheckboxesList(labels);
}

QVector<QPair<int, int>> BandsWidget::get_choosed_bands() {
    QVector<QPair<int, int>> choosed_bands;
    return choosed_bands;
}

void BandsWidget::clear() {
    if (!m_checkboxes.empty()) {
        qDeleteAll(m_checkboxes);
        m_checkboxes.clear();
    }
}

void BandsWidget::onCheckboxStateChanged(QCheckBox* checkBox) {
    if (!checkBox) return;  // Защита от nullptr
    qDebug() << checkBox->text();

    int checkedCount = 0;
    for (QCheckBox* checkbox : qAsConst(m_checkboxes)) {
        if (checkbox->isChecked()) {
            checkedCount++;
        }
    }
}

void BandsWidget::updateCheckboxesList(const QList<QString>& labels) {
    for (const QString& label : labels) {
        QCheckBox* checkbox = new QCheckBox(label, this);
        connect(checkbox, &QCheckBox::toggled, this,
                [this, checkbox]() { onCheckboxStateChanged(checkbox); });
        m_layout->addWidget(checkbox);
        checkbox->setFixedWidth(150);
        m_checkboxes.append(checkbox);
    }
}
