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
    QStringList getLandSat8BandsFromTxtFormat();
    uint16_t* readTiff(const QString& path,
                       int& xSize,
                       int& ySize);

    QString m_root_path;
    QImage m_satellite_image;
    uint16_t* m_landsat8_data_bands[LANDSAT_BANDS_NUMBER];
    QPair<int,int> m_landsat8_bands_image_sizes[LANDSAT_BANDS_NUMBER];
    const QString m_landsat8_bands_keys[LANDSAT_BANDS_NUMBER] = {
     "FILE_NAME_BAND_1",
     "FILE_NAME_BAND_2",
     "FILE_NAME_BAND_3",
     "FILE_NAME_BAND_4",
     "FILE_NAME_BAND_5",
     "FILE_NAME_BAND_6",
     "FILE_NAME_BAND_7",
     "FILE_NAME_BAND_8",
     "FILE_NAME_BAND_9",
     "FILE_NAME_BAND_10",
     "FILE_NAME_BAND_11"
    };
    bool m_is_image_created;
};
#endif // MAIN_WINDOW_SATELLITE_COMPARATOR_H
