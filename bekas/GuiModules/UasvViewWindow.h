#ifndef UASVVIEWWINDOW_H
#define UASVVIEWWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <bekas/GuiModules/SpectrumWidgets/SpectrPlotterWidget.h>
#include <bekas/ProcessingModules/FilesParser.h>

QT_BEGIN_NAMESPACE
namespace Ui { class UasvViewWindow; }
QT_END_NAMESPACE

class UasvViewWindow : public QMainWindow
{
    Q_OBJECT

public:
    UasvViewWindow(QWidget *parent = nullptr);
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


    Ui::UasvViewWindow *ui;                 //!< User interface
    SpectrPlotterWidget *m_plotterWidget;   //!< Plotter widget
    QString m_wTitle = "";                  //!< Main window title
    QSettings *m_settings;                  //!< Settings
    FilesParser *m_filesParser;             //!< The object for files parsing
    QStringListModel *m_slModel;            //!< String list model (for the list of spectra)
};
#endif // UASVVIEWWINDOW_H
