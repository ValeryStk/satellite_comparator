#include "dynamic_checkbox_widget.h"
#include "QDebug"

DynamicCheckboxWidget::DynamicCheckboxWidget(const QList<QString>& labels, QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    for (const QString& label : labels) {
        QCheckBox *checkbox = new QCheckBox(label, this);
        connect(checkbox, &QCheckBox::toggled, this, [this,checkbox](int state){
             onCheckboxStateChanged(checkbox, state);
        });
        layout->addWidget(checkbox);
        checkboxes.append(checkbox);
    }

    setLayout(layout);
}

void DynamicCheckboxWidget::onCheckboxStateChanged(QCheckBox* checkBox,
                                                   int state) {

    if (!checkBox) return; // Защита от nullptr
        qDebug() << checkBox->text();

        // Блокируем сигналы только если это необходимо
        const bool wasBlocked = checkBox->blockSignals(true);
        checkBox->setChecked(false);
        checkBox->blockSignals(wasBlocked); // Восстанавливаем исходное состояние

        /*int checkedCount = 0;
            for (QCheckBox *checkbox : checkboxes) {
                if (checkbox->isChecked()) {
                    checkedCount++;
                }
            }
        if (checkedCount >= maxChecked) {
            changedCheckbox->blockSignals(true);
            changedCheckbox->setChecked(false);
            changedCheckbox->blockSignals(false);
            return;
        }
        qDebug()<<"Checked order size: "<<checkedOrder.size();
        if (changedCheckbox->isChecked()) {
            checkedOrder.append(changedCheckbox);
        } else {
            checkedOrder.removeOne(changedCheckbox);
        }*/
}
