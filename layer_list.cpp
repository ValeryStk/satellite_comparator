#include "layer_list.h"

#include <QApplication>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QDebug>
#include "icon_generator.h"

const char action_show_text[] = "Показать";
const char action_hide_text[] = "Спрятать";
const char action_rename_text[] = "Переименовать";
const char action_delete_text[] = "Удалить";

constexpr int WIDGET_WIDTH = 280;

LayerList::LayerList(QWidget* parent):
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFixedWidth(WIDGET_WIDTH);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint)),this, SLOT(showContextMenu(const QPoint)));
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
    emit removeItem(item);
}

void LayerList::mayBeHideMayBeShow(QListWidgetItem *item)
{
    if (item->flags() & Qt::ItemIsUserCheckable) {
        Qt::CheckState state = item->checkState();
        QVariant nameData = item->data(Qt::UserRole);
        QString id = nameData.toString();
        if(state == Qt::Checked){emit showItem(id);}else {emit hideItem(id);}
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
    QMenu* menu = createContextMenu();
    QAction* selectedAction = menu->exec(viewport()->mapToGlobal(pos));
    if (!selectedAction) return;
    const QString text = selectedAction->text();
    if (text == action_hide_text) {
        item->setCheckState(Qt::CheckState::Unchecked);
    }else if(text == action_show_text){
        item->setCheckState(Qt::CheckState::Checked);
    }
    else if (text == action_delete_text) {
        emit removeItem(item->data(Qt::UserRole).toString());
        delete takeItem(row(item));
    } else if(text == action_rename_text){
        editItem(item);
    } else{
        handle_other_contextAction(text,item);
    }
}

void LayerList::handle_other_contextAction(const QString &actionId,
                                           QListWidgetItem *item)
{
    Q_UNUSED(actionId)
    Q_UNUSED(item)
}

QMenu* LayerList::createContextMenu() {
    QMenu* menu = new QMenu(this);
    menu->addAction(action_show_text);
    menu->addAction(action_hide_text);
    menu->addAction(action_delete_text);
    menu->addAction(action_rename_text);
    return menu;
}

