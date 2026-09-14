#include "layer_search_results_list.h"

#include <QListWidgetItem>
#include <QMenu>

const char action_export_geotiff_text[] = "Экспортировать в GeoTIFF...";

LayerSearchResultsList::LayerSearchResultsList(QWidget *parent)
    : LayerList(parent) {}

QMenu *LayerSearchResultsList::createContextMenu() {
    QMenu *menu = LayerList::createContextMenu();
    menu->addSeparator();
    menu->addAction(action_export_geotiff_text);
    return menu;
}

void LayerSearchResultsList::handle_other_contextAction(const QString &actionId,
                                                        QListWidgetItem *item) {
    if (actionId == action_export_geotiff_text) {
        const QString id = item->data(Qt::UserRole).toString();
        emit exportItemRequested(id);
        return;
    }
}
