#include "dynamic_checkbox_widget.h"
#include "QDebug"
#include <algorithm>

namespace  {
bool isRangeValid(const QVector<int>& vec, const int low, const int max) {
    return std::all_of(vec.begin(), vec.end(), [low,max](int value) {
        return value >= low && value <= max;
    });
}
void clearLayout(QLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater(); // Удаляем виджет
        }
        delete item; // Удаляем сам элемент
    }
}
}

DynamicCheckboxWidget::DynamicCheckboxWidget(const QList<QString>& labels,
                                             QVBoxLayout *layout) {

    for (const QString& label : labels) {
        QCheckBox *checkbox = new QCheckBox(label, this);
        checkbox->setIcon(QIcon(RES_COLOR_BLACK));
        connect(checkbox, &QCheckBox::toggled, this, [this,checkbox](){
             onCheckboxStateChanged(checkbox);
        });
        m_layout = layout;
        layout->addWidget(checkbox);
        checkbox->setFixedWidth(280);
        m_checkboxes.append(checkbox);

    }

}

QVector<QPair<int,int>> DynamicCheckboxWidget::get_choosed_bands()
{
    if(m_checkedOrder.empty())return{};
    QVector<QPair<int,int>> choosed_bands;
    for(auto &info:m_checkedOrder){
        choosed_bands.append({m_checkboxes.indexOf(info.first),info.second});
    }
    return choosed_bands;
}

void DynamicCheckboxWidget::setInitialCheckBoxesToggled(const QVector<int> &toToggle)
{
    if(toToggle.size()!=RGB_NUMBER)return;
    if(isRangeValid(toToggle,0,m_checkboxes.size()-1)==false){
        qDebug()<<"индексы для начальной установки чекбоксов находятся вне допустимого диапазона";
        return;
    };
    m_checkedOrder = {{},{},{}};
    for(int i=0;i<RGB_NUMBER;++i){
        used_colors[i] = true;
        m_checkboxes[toToggle[i]]->blockSignals(true);
        m_checkboxes[toToggle[i]]->setChecked(true);
        m_checkboxes[toToggle[i]]->setIcon(QIcon(colors_svg[i]));
        m_checkedOrder[i] = {m_checkboxes[toToggle[i]],color_sequence[i]};
        m_checkboxes[toToggle[i]]->blockSignals(false);
    }
}

void DynamicCheckboxWidget::setRGBchannels()
{
    for(int i=0;i<m_checkboxes.size();++i){
        m_checkboxes[i]->blockSignals(true);
        m_checkboxes[i]->setChecked(false);
        m_checkboxes[i]->setIcon(QIcon(RES_COLOR_BLACK));
        m_checkboxes[i]->blockSignals(false);
    }
    setInitialCheckBoxesToggled({1,2,3});
}

void DynamicCheckboxWidget::clear()
{
    m_checkboxes.clear();
    m_checkedOrder.clear();
    used_colors[0] = false;
    used_colors[1] = false;
    used_colors[2] = false;
    clearLayout(m_layout);
}

void DynamicCheckboxWidget::onCheckboxStateChanged(QCheckBox* checkBox) {

    if (!checkBox) return; // Защита от nullptr
        qDebug() << checkBox->text();

        int checkedCount = 0;
            for (QCheckBox *checkbox : qAsConst(m_checkboxes)) {
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
            m_checkedOrder.append({checkBox,color});
        } else {
            int color = 0;
            for(int i=0;i<m_checkedOrder.size();++i){
                if(m_checkedOrder[i].first==checkBox){
                    color=m_checkedOrder[i].second;
                    used_colors[color]=false;
                    break;
                }
            }
            m_checkedOrder.removeOne({checkBox,color});
            checkBox->setIcon(QIcon(RES_COLOR_BLACK));
        }
        emit choosed_bands_changed();
}
