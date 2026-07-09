#include "layer_roi_list.h"

#include "QColorDialog"
#include "QDebug"
#include "QMenu"
#include "icon_generator.h"

LayerRoiList::LayerRoiList() {
    connect(this, SIGNAL(itemSelectionChanged()), this,
            SLOT(selectionChanged()));
}

QMenu *LayerRoiList::createContextMenu() {
    auto base_menu = LayerList::createContextMenu();
    base_menu->addAction("Изменить цвет");
    base_menu->addAction("Среднее арифметическое");
    base_menu->addAction("Построить градиент усыхания");
    base_menu->addAction("Градиент усыхания v2");  // ← добавить
    base_menu->addAction("Анализ спектров");
    base_menu->addAction("Метод 'Change detection'");
    return base_menu;
}

void LayerRoiList::handle_other_contextAction(const QString &actionId,
                                              QListWidgetItem *item) {
    if (!item) return;
    QVariant nameData = item->data(Qt::UserRole);
    QString id = nameData.toString();
    if (actionId == "Изменить цвет") {
        QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
        item->setIcon(
            iut::createIcon(color.red(), color.green(), color.blue()));
        emit roi_color_changed(id, color);
    } else if (actionId == "Среднее арифметическое") {
        qDebug() << "average...";
        emit roiPolygonAverage(id);
    } else if (actionId == "Построить градиент усыхания") {
        emit createTimeRowGradient(id);
    } else if (actionId == "Градиент усыхания v2") {
        emit createTimeRowIndexesGradient(id);
    } else if (actionId == "Анализ спектров") {
        emit polygonForMatlabSelected(id);
    } else if (actionId == "Метод 'Change detection'") {
        emit changeDetectionRegion(id);
    }
}

void LayerRoiList::selectionChanged() {
    QList<QListWidgetItem *> selectedItems = this->selectedItems();
    QListWidgetItem *selected_item;
    if (selectedItems.empty()) return;
    for (QListWidgetItem *item : qAsConst(selectedItems)) {
        selected_item = item;
        break;
    }
    QVariant nameData = selected_item->data(Qt::UserRole);
    const QString id = nameData.toString();
    emit roi_item_selected(id);
}
