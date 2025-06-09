#ifndef PHOTOSPECTRALGRAPHICSVIEW_H
#define PHOTOSPECTRALGRAPHICSVIEW_H

#include <QGraphicsView>

namespace Ui {
class PhotospectralGraphicsView;
}

class PhotospectralGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit PhotospectralGraphicsView(QWidget *parent = nullptr);
    ~PhotospectralGraphicsView() override;

    void setShowingImage(QString imageFilePath, QVector<QPair<int,int>> spFov);

    void saveImageWithFov(QString savingFilePath);

private slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    void paintImageAndFov();

    QPolygonF getPolygonFromPoints(QVector<QPair<int, int> > fovPoints);

    QPair<int, int> recalcPointCoords(QPair<int, int> baseCoords, QSize baseSize);

    QGraphicsScene *m_scene;                    //!< For Image with FOV
    QString m_imageFilepath;
    QVector<QPair<int, int> > m_specFov;
};

#endif // PHOTOSPECTRALGRAPHICSVIEW_H
