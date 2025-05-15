#include "progress_informator.h"
#include "QGuiApplication"
#include <QScreen>

ProgressInformator::ProgressInformator(QWidget *parent, const QString& text)
{
    this->setParent(parent);
    this->setText(text);
    this->setStyleSheet("background-color: white; border: 1px solid black;");
    QFont font("Arial", 16, QFont::Bold);
    this->setFont(font);
    this->setText("Пожалуйста подождите,\nпроисходит поиск областей\n(Евклидова метрика)...");
    this->adjustSize();
    int x = (parent->width() - this->width()) / 2;
    int y = (parent->height() - this->height()) / 2;
    this->move(x, y);
}
