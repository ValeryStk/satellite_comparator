#include "dynamic_checkbox_widget.h"
#include "QDebug"
#include <algorithm>

namespace  {
bool isRangeValid(const QVector<int>& vec, const int low, const int max) {
    return std::all_of(vec.begin(), vec.end(), [low,max](int value) {
        return value >= low && value <= max;
    });
}
}

DynamicCheckboxWidget::DynamicCheckboxWidget(const QList<QString>& labels,
                                             QWidget *parent,
                                             QVBoxLayout *layout)
    : QWidget(parent) {

    for (const QString& label : labels) {
        QCheckBox *checkbox = new QCheckBox(label, this);
        checkbox->setIcon(QIcon(RES_COLOR_BLACK));
        connect(checkbox, &QCheckBox::toggled, this, [this,checkbox](){
             onCheckboxStateChanged(checkbox);
        });
        layout->addWidget(checkbox);
        checkboxes.append(checkbox);
    }

    setLayout(layout);
}

QVector<QPair<int,int>> DynamicCheckboxWidget::get_choosed_bands()
{
    if(checkedOrder.empty())return{};
    QVector<QPair<int,int>> choosed_bands;
    for(auto &info:checkedOrder){
        choosed_bands.append({checkboxes.indexOf(info.first),info.second});
    }
    return choosed_bands;
}

void DynamicCheckboxWidget::setInitialCheckBoxesToggled(const QVector<int> &toToggle)
{
    if(toToggle.size()!=RGB_NUMBER)return;
    if(isRangeValid(toToggle,0,checkboxes.size()-1)==false){
        qDebug()<<"индексы для начальной установки чекбоксов находятся вне допустимого диапазона";
        return;
    };
    checkedOrder = {{},{},{}};
    for(int i=0;i<RGB_NUMBER;++i){
        used_colors[i] = true;
        checkboxes[toToggle[i]]->blockSignals(true);
        checkboxes[toToggle[i]]->setChecked(true);
        checkboxes[toToggle[i]]->setIcon(QIcon(colors_svg[i]));
        checkedOrder[i] = {checkboxes[toToggle[i]],color_sequence[i]};
        checkboxes[toToggle[i]]->blockSignals(false);
    }
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
        if (checkedCount > MAX_POSSIBLE_CHECKED) {
            checkBox->blockSignals(true);
            checkBox->setChecked(false);
            checkBox->blockSignals(false);
            return;
        }

        if (checkBox->isChecked()) {
            int color=0;
            if(used_colors[RED]==false){
              checkBox->setIcon(QIcon(RES_COLOR_RED));
              used_colors[RED]=true;
              color=RED;
            }else if(used_colors[GREEN]==false){
              checkBox->setIcon(QIcon(RES_COLOR_GREEN));
              used_colors[GREEN]=true;
              color=GREEN;
            }else if(used_colors[BLUE]==false){
              checkBox->setIcon(QIcon(RES_COLOR_BLUE));
              used_colors[BLUE]=true;
              color=BLUE;
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
            checkBox->setIcon(QIcon(RES_COLOR_BLACK));
        }
        emit choosed_bands_changed();
}
