#ifndef LAYER_LIST_H
#define LAYER_LIST_H

#include <QListWidget>

extern const char action_show_text[];
extern const char action_hide_text[];
extern const char action_rename_text[];
extern const char action_delete_text[];


class LayerList:public QListWidget
{
    Q_OBJECT
public:
    explicit LayerList(QWidget* parent = nullptr);
    void addItemToList(const QString& item,
                       const QString& toolTip,
                       const QColor& color);

    void removeItemList(const QString& item);
    void mayBeHideMayBeShow(QListWidgetItem* item);
    void deleteAllItems();

    virtual QMenu* createContextMenu();


private slots:
    void showContextMenu(const QPoint& pos);


private:
    virtual void handle_other_contextAction(const QString& actionId, QListWidgetItem* item);
    QHash<QString,bool> m_states;

signals:
    void hideItem(const QString);
    void showItem(const QString);
    void removeItem(const QString);
};

#endif // LAYER_LIST_H
