#ifndef UASVVIEWWINDOW_H
#define UASVVIEWWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <bekas/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h>
#include <bekas/ProcessingModules/FilesParser.h>
#include "udpjsonrpc.h"
#include <QStringListModel>
#include <QHash>
#include <QColor>
#include <QBrush>

QT_BEGIN_NAMESPACE
namespace Ui { class UasvViewWindow; }
QT_END_NAMESPACE

class CustomStringListModel : public QStringListModel
{
    Q_OBJECT
public:
    explicit CustomStringListModel(QObject *parent = nullptr);
    explicit CustomStringListModel(const QStringList &strings, QObject *parent = nullptr);

    // Установка цвета по имени
    void setColorForName(const QString& name, const QColor& color);
    // Очистка цвета для имени
    void clearColorForName(const QString& name);
    // Получение цвета по имени
    QColor getColorForName(const QString& name) const;

    // Установка кластера для имени
    void setClusterForName(const QString& name, int cluster);
    // Очистка кластера для имени
    void clearClusterForName(const QString& name);
    // Получение кластера по имени
    int getClusterForName(const QString& name) const;

    // Сброс всех кластеров и цветов
    void resetAllHighlights();

    // Переопределение метода data
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QHash<QString, QColor> m_colorMap;
    QHash<QString, int> m_clusterMap; // Хранит номера кластеров

    void updateAllAppearances(const QString& name);
};




class UasvViewWindow : public QMainWindow
{
    Q_OBJECT
    friend class bekas_UnitTests;

public:
    UasvViewWindow(QWidget *parent = nullptr);
    void setUDPobj(UdpJsonRpc *rpc);
    void updateListWithClustNums(const QStringList &specNames,
                                 const QVector<int>  &clusters,
                                 const QVector<QColor> &colorsOfEachSpectr);


    ~UasvViewWindow();

private slots:
    void on_pushButtonChangeInputFolder_clicked();

    void on_pushButtonShowRfl_clicked();

    void on_pushButtonShowPattBright_clicked();

    void on_pushButtonShowTemplBright_clicked();

    void on_pushButtonSaveChanges_clicked();

    void on_pushButtonSaveGraph_clicked();

    void on_pushButtonExportAsText_clicked();

    void on_pushButtonSetWaveRange_clicked();

    void on_pushButtonPrevious_clicked();

    void on_pushButtonNext_clicked();

    void on_listViewSpectraNames_activated(const QModelIndex &index);

    void keyPressEvent(QKeyEvent* event);

    void keyReleaseEvent(QKeyEvent* event);

    void on_pushButtonOpenSavingFolder_clicked();

    void on_pushButtonToMatlab_clicked();

signals:
    void sendSampleForSatelliteComparator(QVector<double> x, QVector<double> y);

private:
    /**
     * @brief setupProject   Function that setups project
     */
    void setupProject();

    /**
     * @brief setupGui  Function that setups GUI
     */
    void setupGui();

    /**
     * @brief initObjects   Function that inits objects
     */
    void initObjects();


    /**
     * @brief parseInputFolder  Function to find all the spectra in start directory
     */
    void parseInputFolder();

    void updateDataInGui();

    void fillListOfSpectra();

    void drawSpectrum(QVector<double> &waves, QVector<double> &values, double maxValue,
                      db_json::BandUnits bandsUnits, db_json::SpectrumUnits spUnits, QString spName);

    void fillMetaData(db_json::META_DATA spectrumMd);

    QString getSpecFileNameSuffix(int &currSpecIndex);

    void checkClassifAndSetupCBox(QString spClassification, QComboBox *settingCBox, QString textForEmpty);

    void applySpectraColorsWithClusterNumbers(QStringListModel *model,
                                              const QStringList &specNames,
                                              const QVector<int>  &clusters,
                                              const QVector<QColor> &colorsOfEachSpectr);


    Ui::UasvViewWindow *ui;                 //!< User interface
    SpectrPlotterWidget *m_plotterWidget;   //!< Plotter widget
    QString m_wTitle = "";                  //!< Main window title
    QSettings *m_settings;                  //!< Settings
    FilesParser *m_filesParser;             //!< The object for files parsing
    CustomStringListModel *m_slModel;       //!< String list model (for the list of spectra)
    UdpJsonRpc   *m_rpc;
    QStringList m_listOfOriginSpectraNames;
};
#endif // UASVVIEWWINDOW_H
