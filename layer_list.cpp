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
    setFixedWidth(280);
    connect(this, &QListWidget::customContextMenuRequested,
            this, &LayerList::showContextMenu);
}

void LayerList::addItemToList(const QString &itemName,
                              const QString& toolTip,
                              const QColor& color)
{
    QListWidgetItem* item = new QListWidgetItem(itemName);
    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);  // Разрешаем редактирование
    item->setCheckState(Qt::Checked);
    item->setData(Qt::UserRole,itemName);
    addItem(item);
    item->setToolTip(toolTip);
    item->setIcon(iut::createIcon(color.red(),color.green(),color.blue()));
    connect(this, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
        mayBeHideMayBeShow(item);
    });


}

void LayerList::removeItemList(const QString &item)
{
    emit remove(item);
}

void LayerList::mayBeHideMayBeShow(QListWidgetItem *item)
{
    if (item->flags() & Qt::ItemIsUserCheckable) {
        Qt::CheckState state = item->checkState();
        QVariant nameData = item->data(Qt::UserRole);
        QString id = nameData.toString();
        if(state == Qt::Checked){emit show(id);}else {emit hide(id);}
    }
}

void LayerList::deleteAllItems()
{
    while (count() > 0) {
        delete takeItem(0);
    }
}

void LayerList::showContextMenu(const QPoint &pos)
{
    QListWidgetItem* item = itemAt(pos);
    if (!item) return;
    QMenu menu(this);
    QAction* showAction = menu.addAction("Показать");
    QAction* hideAction = menu.addAction("Спрятать");
    QAction* deleteAction = menu.addAction("Удалить");
    QAction* renameAction = menu.addAction("Переименовать");

    QAction* selectedAction = menu.exec(viewport()->mapToGlobal(pos));
    if (selectedAction == hideAction) {
      item->setCheckState(Qt::CheckState::Unchecked);
    }else if(selectedAction == showAction){
       item->setCheckState(Qt::CheckState::Checked);
    }
    else if (selectedAction == deleteAction) {
        emit remove(item->data(Qt::UserRole).toString());
        delete takeItem(row(item));
    } else if(selectedAction == renameAction){
        editItem(item);
    }
}

