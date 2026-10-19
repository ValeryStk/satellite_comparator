#include "layer_roi_list.h"

#include <QColorDialog>
#include <QMenu>

#include "icon_generator.h"

namespace {

const QString STR_CHNG_COLOR("Изменить цвет");
const QString STR_AVG("Среднее арифметическое");
const QString STR_AVG_CATI("Среднее арифметическое CATI");
const QString STR_AVG_Sen2Cor("Среднее арифметическое Sen2Cor");
const QString STR_AVG_base_CATI("Установить средний СПЭЯ для CATI");
const QString STR_SAVE_CATI_as_GEOTIFF("Сохранить CATI как Geo Tiff");
const QString STR_GRADIENT_V1("Градиент усыхания");
const QString STR_GRADIENT_INDEXES("Градиент усыхания по индексам");
const QString STR_SPETRA_PLOTTER("Анализ спектров");
const QString STR_CHANG_DETECTION("Метод 'Change detection'");
const QString STR_INDEXES_DYNAMICS("Расчет динамики индексов");
const QString STR_CLASSIFY_INDEXES("Классификация по индексам");

}  // namespace

LayerRoiList::LayerRoiList() {
    connect(this, &QListWidget::itemSelectionChanged, this,
            &LayerRoiList::selectionChanged);
}

QMenu *LayerRoiList::createContextMenu() {
    auto *base_menu = LayerList::createContextMenu();

    base_menu->addAction(STR_CHNG_COLOR);
    base_menu->addAction(STR_AVG);
    base_menu->addAction(STR_AVG_CATI);
    base_menu->addAction(STR_AVG_Sen2Cor);
    base_menu->addAction(STR_AVG_base_CATI);

    // Как и раньше, экспорт CATI здесь не показываем.
    // base_menu->addAction(STR_SAVE_CATI_as_GEOTIFF);

    base_menu->addAction(STR_CLASSIFY_INDEXES);
    base_menu->addAction(STR_GRADIENT_V1);
    base_menu->addAction(STR_GRADIENT_INDEXES);
    base_menu->addAction(STR_INDEXES_DYNAMICS);
    base_menu->addAction(STR_SPETRA_PLOTTER);
    base_menu->addAction(STR_CHANG_DETECTION);

    return base_menu;
}

void LayerRoiList::handle_other_contextAction(const QString &actionId,
                                              QListWidgetItem *item) {
    if (!item) {
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();

    if (actionId == STR_CHNG_COLOR) {
        const QColor color =
            QColorDialog::getColor(Qt::white, this, "Выберите цвет");

        if (!color.isValid()) {
            return;
        }

        item->setIcon(
            iut::createIcon(color.red(), color.green(), color.blue()));

        emit roi_color_changed(id, color);

    } else if (actionId == STR_AVG) {
        emit roiPolygonAverage(id);

    } else if (actionId == STR_AVG_CATI) {
        emit roiPolygonAverageCATI(id);

    } else if (actionId == STR_AVG_Sen2Cor) {
        emit roiPolygonAverageSen2Cor(id);

    } else if (actionId == STR_AVG_base_CATI) {
        emit roiPolygonAverageBaseCATI(id);

    } else if (actionId == STR_SAVE_CATI_as_GEOTIFF) {
        emit saveCATIasGeoTiff(id);

    } else if (actionId == STR_CLASSIFY_INDEXES) {
        emit classifyRoiByIndexes(id);

    } else if (actionId == STR_GRADIENT_V1) {
        emit createTimeRowGradient(id);

    } else if (actionId == STR_GRADIENT_INDEXES) {
        emit createTimeRowIndexesGradient(id);

    } else if (actionId == STR_INDEXES_DYNAMICS) {
        emit createTimeRowIndexesDynamics(id);

    } else if (actionId == STR_SPETRA_PLOTTER) {
        emit polygonForMatlabSelected(id);

    } else if (actionId == STR_CHANG_DETECTION) {
        emit changeDetectionRegion(id);
    }
}

void LayerRoiList::selectionChanged() {
    const QList<QListWidgetItem *> selected = selectedItems();

    if (selected.isEmpty()) {
        return;
    }

    const QString id = selected.first()->data(Qt::UserRole).toString();

    emit roi_item_selected(id);
}
