#ifndef LAYER_SEARCH_RESULTS_LIST_H
#define LAYER_SEARCH_RESULTS_LIST_H

#include "layer_list.h"

extern const char action_export_geotiff_text[];

// Список в панели "Результаты поиска" (маски усыхания, поиск по метрике,
// MATLAB-классификация). Расширяет базовый LayerList единственным
// дополнительным пунктом контекстного меню - экспортом выбранного результата
// в GeoTIFF. Реализовано через уже существующий в LayerList механизм
// расширения (handle_other_contextAction), чтобы не менять поведение
// базового класса и LayerRoiList, который от него тоже наследуется.
class LayerSearchResultsList : public LayerList {
    Q_OBJECT
public:
    explicit LayerSearchResultsList(QWidget *parent = nullptr);

    QMenu *createContextMenu() override;

signals:
    // id - тот же идентификатор, что хранится в m_layers_search_result_items
    // и был передан в addItemToList()
    void exportItemRequested(const QString &id);

private:
    void handle_other_contextAction(const QString &actionId,
                                     QListWidgetItem *item) override;
};

#endif // LAYER_SEARCH_RESULTS_LIST_H
