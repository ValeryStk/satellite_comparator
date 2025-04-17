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


        int checkedCount = 0;
            for (QCheckBox *checkbox : checkboxes) {
                if (checkbox->isChecked()) {
                    checkedCount++;
                }
            }
        if (checkedCount > maxChecked) {
            checkBox->blockSignals(true);
            checkBox->setChecked(false);
            checkBox->blockSignals(false);
            return;
        }
        qDebug()<<"Checked order size: "<<checkedOrder.size();
        if (checkBox->isChecked()) {
            checkedOrder.append(checkBox);
        } else {
            checkedOrder.removeOne(checkBox);
        }
}
