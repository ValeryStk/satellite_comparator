#include "layer_list.h"

#include <QApplication>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QDebug>
#include "icon_generator.h"

LayerList::LayerList(QWidget* parent):
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested,
            this, &LayerList::showContextMenu);
}

void LayerList::addItemToList(const QString &itemName, const QColor& color)
{
    QListWidgetItem* item = new QListWidgetItem(itemName);
    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);  // Разрешаем редактирование
    item->setCheckState(Qt::Checked);
    addItem(item);
    item->setIcon(iut::createIcon(color.red(),color.green(),color.blue(),color.alpha()));
    connect(this, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
        if (item->flags() & Qt::ItemIsUserCheckable) {
            Qt::CheckState state = item->checkState();
            auto id = item->text();
            state == Qt::Checked?emit show(id):emit hide(id);
        }
    });


}

void LayerList::removeItemList(const QString &item)
{
    remove(item);
}

void LayerList::showContextMenu(const QPoint &pos)
{
    QListWidgetItem* item = itemAt(pos);
    if (!item) return;
    QMenu menu(this);
    QAction* hideAction = menu.addAction("Спрятать");
    QAction* deleteAction = menu.addAction("Удалить");
    QAction* renameAction = menu.addAction("Переименовать");

    QAction* selectedAction = menu.exec(viewport()->mapToGlobal(pos));
    if (selectedAction == hideAction) {
        item->text();
        item->setForeground(QColor(50, 50, 50));
    } else if (selectedAction == deleteAction) {
        emit remove(item->text());
        delete takeItem(row(item));
    } else if(selectedAction == renameAction){
        editItem(item);
    }
}

void LayerList::setupContextMenu()
{

}
