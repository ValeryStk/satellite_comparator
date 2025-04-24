#ifndef MAIN_WINDOW_SATELLITE_COMPARATOR_H
#define MAIN_WINDOW_SATELLITE_COMPARATOR_H

#include <QMainWindow>
#include <cstdint>
#include <sattelite_comparator.h>
#include "dynamic_checkbox_widget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowSatelliteComparator; }
QT_END_NAMESPACE

#define LANDSAT_BANDS_NUMBER 11

class MainWindowSatelliteComparator : public QMainWindow
{
    Q_OBJECT

public:
    MainWindowSatelliteComparator(QWidget *parent = nullptr);
    ~MainWindowSatelliteComparator();

private slots:
    void on_pushButton_open_sat_header_clicked();
    void change_bands_and_show_image();

private:
    Ui::MainWindowSatelliteComparator *ui;
    DynamicCheckboxWidget *m_dynamic_checkboxes_widget;
    SatteliteComparator* m_sat_comparator;
    void openHeaderData();
    uint16_t* readTiff(const QString& path,
                       int& xSize,
                       int& ySize);

    QList<QString> m_band_names;
    QString m_root_path;
    QImage m_satellite_image;
    uint16_t* m_landsat8_bands[LANDSAT_BANDS_NUMBER];
    QPair<int,int> m_landsat8_bands_image_sizes[LANDSAT_BANDS_NUMBER];
    bool m_is_image_created;
};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
