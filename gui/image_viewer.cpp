#include "image_viewer.h"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPixmap>

ImageViewer::ImageViewer(QImage& image, const QString& windowTitle, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(windowTitle);
    setFixedSize(600, 400); // фиксированный размер окна
    setAttribute(Qt::WA_DeleteOnClose); // автоматическое удаление после закрытия

    imageLabel = new QLabel;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->setAlignment(Qt::AlignCenter);

    scrollArea = new QScrollArea;
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);

    layout = new QVBoxLayout;
    layout->addWidget(scrollArea);
    setLayout(layout);
}

ImageViewer::~ImageViewer() {
    // Все ресурсы удаляются автоматически благодаря родительской иерархии
}
