#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QWidget>
#include <QImage>
#include <QString>

class QLabel;
class QScrollArea;
class QVBoxLayout;

class ImageViewer : public QWidget {
    Q_OBJECT

public:
    explicit ImageViewer(QImage &image,
                         const QString& windowTitle,
                         QWidget* parent = nullptr);
    ~ImageViewer();

private:
    QLabel* imageLabel;
    QScrollArea* scrollArea;
    QVBoxLayout* layout;
};

#endif // IMAGEVIEWER_H
