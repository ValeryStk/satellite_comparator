#include "layer_roi_list.h"

#include "QColorDialog"
#include "QDebug"
#include "QMenu"
#include "icon_generator.h"

namespace {

const QString STR_CHNG_COLOR("Изменить цвет");
const QString STR_AVG("Среднее арифметическое");
const QString STR_GRADIENT_V1("Градиент усыхания");
const QString STR_GRADIENT_INDEXES("Градиент усыхания по индексам");
const QString STR_SPETRA_PLOTTER("Анализ спектров");
const QString STR_CHANG_DETECTION("Метод 'Change detection'");
}  // namespace

LayerRoiList::LayerRoiList() {
    connect(this, SIGNAL(itemSelectionChanged()), this,
            SLOT(selectionChanged()));
}

QMenu *LayerRoiList::createContextMenu() {
    auto base_menu = LayerList::createContextMenu();
    base_menu->addAction(STR_CHNG_COLOR);
    base_menu->addAction(STR_AVG);
    base_menu->addAction(STR_GRADIENT_V1);
    base_menu->addAction(STR_GRADIENT_INDEXES);
    base_menu->addAction(STR_SPETRA_PLOTTER);
    base_menu->addAction(STR_CHANG_DETECTION);
    return base_menu;
}

void LayerRoiList::handle_other_contextAction(const QString &actionId,
                                              QListWidgetItem *item) {
    if (!item) return;
    QVariant nameData = item->data(Qt::UserRole);
    QString id = nameData.toString();
    if (actionId == STR_CHNG_COLOR) {
        QColor color = QColorDialog::getColor(Qt::white, this, "Выберите цвет");
        item->setIcon(
            iut::createIcon(color.red(), color.green(), color.blue()));
        emit roi_color_changed(id, color);
    } else if (actionId == STR_AVG) {
        qDebug() << "average...";
        emit roiPolygonAverage(id);
    } else if (actionId == STR_GRADIENT_V1) {
        emit createTimeRowGradient(id);
    } else if (actionId == STR_GRADIENT_INDEXES) {
        emit createTimeRowIndexesGradient(id);
    } else if (actionId == STR_SPETRA_PLOTTER) {
        emit polygonForMatlabSelected(id);
    } else if (actionId == STR_CHANG_DETECTION) {
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
