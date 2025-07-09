#ifndef LAYER_LIST_H
#define LAYER_LIST_H

#include <QListWidget>

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


private slots:
    void showContextMenu(const QPoint& pos);

private:
    QHash<QString,bool> m_states;
    void setupContextMenu();

signals:
    void hide(const QString);
    void show(const QString);
    void remove(const QString);
};

#endif // LAYER_LIST_H
