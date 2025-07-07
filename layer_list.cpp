#include "layer_list.h"

#include <QApplication>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>

LayerList::LayerList(QWidget* parent):
    QListWidget(parent)
{
       setContextMenuPolicy(Qt::CustomContextMenu);
       connect(this, &QListWidget::customContextMenuRequested,
               this, &LayerList::showContextMenu);
}

void LayerList::addItemToList(const QString &itemName)
{
    QListWidgetItem* item = new QListWidgetItem(itemName);
    item->setFlags(item->flags() | Qt::ItemIsEditable);  // Разрешаем редактирование
    addItem(item);

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
