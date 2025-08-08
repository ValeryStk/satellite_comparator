#include "layer_roi_list.h"

#include "QMenu"
#include "QDebug"
#include "QColorDialog"
#include "icon_generator.h"

LayerRoiList::LayerRoiList()
{

}

QMenu *LayerRoiList::createContextMenu()
{
    auto base_menu = LayerList::createContextMenu();
    base_menu->addAction("Изменить цвет");
    return base_menu;
}

void LayerRoiList::handle_other_contextAction(const QString &actionId,
                                              QListWidgetItem *item)
{
    if (!item) return;
    if(actionId == "Изменить цвет"){
        QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
        item->setIcon(iut::createIcon(color.red(),color.green(),color.blue()));
        QVariant nameData = item->data(Qt::UserRole);
        QString id = nameData.toString();
        emit roi_color_changed(id,color);
    }
}

