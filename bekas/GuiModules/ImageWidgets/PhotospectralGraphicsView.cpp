#include "PhotospectralGraphicsView.h"

PhotospectralGraphicsView::PhotospectralGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    m_scene = new QGraphicsScene();
    this->setScene(m_scene);
}

PhotospectralGraphicsView::~PhotospectralGraphicsView()
{

}

void PhotospectralGraphicsView::setShowingImage(QString imageFilePath, QVector<QPair<int, int> > spFov)
{
    m_scene->clear();
    m_imageFilepath = imageFilePath;
    m_specFov = spFov;
    paintImageAndFov();
}

void PhotospectralGraphicsView::saveImageWithFov(QString savingFilePath)
{
    QImage fullImage = QImage(m_imageFilepath, "PNG");
    QGraphicsScene scene;
    scene.addPixmap(QPixmap::fromImage(fullImage));
    scene.addPolygon(getPolygonFromPoints(m_specFov),
                     QPen(QBrush(QColor(242, 196, 44)), 1),
                     QBrush(QColor(250, 220, 96, 60)));
    scene.setSceneRect(scene.itemsBoundingRect());
    QImage image(scene.sceneRect().size().toSize(), QImage::Format_RGB666);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    scene.render(&painter);
    QPixmap::fromImage(image).save(savingFilePath);
}

void PhotospectralGraphicsView::resizeEvent(QResizeEvent *event)
{
    m_scene->clear();
    paintImageAndFov();
    QGraphicsView::resizeEvent(event);
}

void PhotospectralGraphicsView::paintImageAndFov()
{
    if(!m_imageFilepath.isEmpty()){
        QImage fullImage = QImage(m_imageFilepath, "PNG");
        QImage image = fullImage.scaled(QSize(this->width(), this->height()),
                                              Qt::IgnoreAspectRatio, Qt::FastTransformation);
        m_scene->addPixmap(QPixmap::fromImage(image));

        QVector<QPair<int,int>> scaledFov;
        for(int i = 0; i < m_specFov.count(); i++){
            QPair<int, int> currentPoint = recalcPointCoords(m_specFov.at(i), fullImage.size());
            scaledFov.append(currentPoint);
        }
        m_scene->addPolygon(getPolygonFromPoints(scaledFov),
                            QPen(QBrush(QColor(242, 0, 0)), 1),
                            QBrush(QColor(250, 220, 96, 60)));

        m_scene->setSceneRect(m_scene->itemsBoundingRect());
        m_scene->update();
    }else{
        m_scene->addSimpleText("Изображение отсутствует", QFont("Helvetica", 16));
    }
}

QPolygonF PhotospectralGraphicsView::getPolygonFromPoints(QVector<QPair<int, int> > fovPoints)
{
    QVector<QPointF> points;
    for(int i = 0; i < fovPoints.count(); i++){
        QPointF point;
        point.rx() = fovPoints.at(i).first;
        point.ry() = fovPoints.at(i).second;
        points.append(point);
    }
    return QPolygonF(points);
}

QPair<int, int> PhotospectralGraphicsView::recalcPointCoords(QPair<int, int> baseCoords, QSize baseSize)
{
    QPair<int, int> scaledPoint;
    QSize currentSize = QSize(this->width(), this->height());    
    scaledPoint.first = static_cast<double>(this->width()) * baseCoords.first / baseSize.width();
    scaledPoint.second = static_cast<double>(this->height()) * baseCoords.second / baseSize.height();
    return scaledPoint;
}
