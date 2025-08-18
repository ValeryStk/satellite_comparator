#ifndef LAYERROILIST_H
#define LAYERROILIST_H

#include <QObject>
#include "layer_list.h"

class LayerRoiList:public LayerList
{
    Q_OBJECT
public:
    LayerRoiList();
    QMenu* createContextMenu() override;


signals:
    void roi_color_changed(const QString& id, const QColor& color);
    void roi_item_selected(const QString& id);
    void roiPolygonAverage(const QString& id);

    // LayerList interface
private:
    void handle_other_contextAction(const QString &actionId, QListWidgetItem *item) override;

private slots:
    void selectionChanged();

};

#endif // LAYERROILIST_H
