#include "dynamic_checkbox_widget.h"
#include "QDebug"

DynamicCheckboxWidget::DynamicCheckboxWidget(const QList<QString>& labels, QWidget *parent)
    : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    for (const QString& label : labels) {
        QCheckBox *checkbox = new QCheckBox(label, this);
        checkbox->setIcon(QIcon(":/res/white.svg"));
        connect(checkbox, &QCheckBox::toggled, this, [this,checkbox](){
             onCheckboxStateChanged(checkbox);
        });
        layout->addWidget(checkbox);
        checkboxes.append(checkbox);
    }

    setLayout(layout);
}

void DynamicCheckboxWidget::onCheckboxStateChanged(QCheckBox* checkBox) {

    if (!checkBox) return; // Защита от nullptr
        qDebug() << checkBox->text();


        int checkedCount = 0;
            for (QCheckBox *checkbox : qAsConst(checkboxes)) {
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
            int color=0;
            if(used_colors[0]==false){
              checkBox->setIcon(QIcon(":/res/red.svg"));
              used_colors[0]=true;
              color=0;
            }else if(used_colors[1]==false){
              checkBox->setIcon(QIcon(":/res/green.svg"));
              used_colors[1]=true;
              color=1;
            }else if(used_colors[2]==false){
              checkBox->setIcon(QIcon(":/res/blue.svg"));
              used_colors[2]=true;
              color=2;
            }
            checkedOrder.append({checkBox,color});
        } else {
            int color = 0;
            for(int i=0;i<checkedOrder.size();++i){
                if(checkedOrder[i].first==checkBox){
                    color=checkedOrder[i].second;
                    used_colors[color]=false;
                    break;
                }
            }
            checkedOrder.removeOne({checkBox,color});
            checkBox->setIcon(QIcon(":/res/white.svg"));
        }
}
