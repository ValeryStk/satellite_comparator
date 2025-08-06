#include "UasvViewWindow.h"
#include "ui_UasvViewWindow.h"
#include <bekas/BaseTools/IniFileLoader.h>
#include "bekas/version.h"
#include <QMessageBox>
#include <bekas/GuiModules/SpectrumWidgets/WavesRangeDialog.h>
#include <bekas/ProcessingModules/SpectrDataSaver.h>
#include "text_constants.h"

UasvViewWindow::UasvViewWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UasvViewWindow)
{
    ui->setupUi(this);

    ui->widgetSpectra->setContextMenuPolicy(Qt::CustomContextMenu);
    QMenu *menu = new QMenu(this);
    QAction* sendAsSample = new QAction(satc::action_send_sample_text,this);
    menu->addAction(sendAsSample);
    connect(sendAsSample, &QAction::triggered, this, [this](){
        double maxInSpectrum;
        QVector<double> waves = m_filesParser->getRflWaves();
        QVector<double> values = m_filesParser->getRflSpectrumValues(maxInSpectrum);
        emit sendSampleForSatelliteComparator(waves,values);
    });
    connect(ui->widgetSpectra, &QCustomPlot::customContextMenuRequested, this, [menu](const QPoint &pos){
        menu->exec(QCursor::pos());
    });


    setupProject();
    initObjects();
    setupGui();
    parseInputFolder();

    double startShowingWave = m_settings->value("Settings/ShowingStartWave").toDouble();
    double endShowingWave = m_settings->value("Settings/ShowingEndWave").toDouble();
    if(!m_plotterWidget->bands().isEmpty()){
        m_plotterWidget->setBandsRange(startShowingWave, endShowingWave);
    }
}

UasvViewWindow::~UasvViewWindow()
{
    delete ui;
    delete m_filesParser;
    delete m_settings;
}

void UasvViewWindow::setupProject()
{
    m_wTitle.append(VER_PRODUCTNAME_STR).append(" v_").append(VER_FILEVERSION_STR);
    this->setWindowTitle(m_wTitle);
    m_settings = IniFileLoader::createSettingsObject(VER_PRODUCTNAME_STR);
}

void UasvViewWindow::setupGui()
{
    m_plotterWidget = new SpectrPlotterWidget(ui->widgetSpectra);
    m_slModel = new QStringListModel(this);
    ui->listViewSpectraNames->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void UasvViewWindow::initObjects()
{
    ui->lineEditInputPath->setText(m_settings->value("Pathes/InputPath").toString());
    m_filesParser = new FilesParser(this, ui->lineEditInputPath->text());
}


void UasvViewWindow::parseInputFolder()
{
    if(!ui->lineEditInputPath->text().isEmpty()){
        m_filesParser->parseInputDir();
        fillListOfSpectra();
        updateDataInGui();
    }
}

void UasvViewWindow::updateDataInGui()
{
    if(m_slModel->stringList().count() > 0){
        db_json::META_DATA spectrumMd = m_filesParser->getCurrentMetadata();
        fillMetaData(spectrumMd);

        double maxInSpectrum;
        QVector<double> waves = m_filesParser->getRflWaves();
        QVector<double> values = m_filesParser->getRflSpectrumValues(maxInSpectrum);
        drawSpectrum(waves, values, maxInSpectrum, db_json::BU_WAVELENGTH, db_json::SU_RFL, m_filesParser->currentSpName());
        ui->pushButtonShowRfl->setChecked(true);
        ui->pushButtonShowPattBright->setChecked(false);
        ui->pushButtonShowTemplBright->setChecked(false);

        QVector<QPair<int,int>> spFov;
        QString imageFilePath = m_filesParser->getImagePath(spFov);
        ui->graphicsViewImage->setShowingImage(imageFilePath, spFov);

    }
}

void UasvViewWindow::fillListOfSpectra()
{
    QStringList listOfSpectrums = m_filesParser->spectraNamesList();
    if(!m_slModel->stringList().isEmpty()){
        m_slModel->removeRows(0, m_slModel->stringList().count());
    }
    m_slModel->setStringList(listOfSpectrums);
    ui->listViewSpectraNames->setModel(m_slModel);

    if(!m_slModel->stringList().isEmpty())
        ui->listViewSpectraNames->setCurrentIndex(ui->listViewSpectraNames->indexAt(QPoint(0,0)));
}

void UasvViewWindow::drawSpectrum(QVector<double> &waves,
                                  QVector<double> &values,
                                  double maxValue,
                                  db_json::BandUnits bandsUnits,
                                  db_json::SpectrumUnits spUnits,
                                  QString spName)
{
    //qDebug()<<waves.count()<<values.count()<<spName;
    m_plotterWidget->setBands(waves, bandsUnits);
    m_plotterWidget->setGraphValues(spUnits);
    m_plotterWidget->showSpectrum(values, maxValue, false, spName);
}

void UasvViewWindow::fillMetaData(db_json::META_DATA spectrumMd)
{
    ui->labelSpectrumName->setText(spectrumMd.owner + ": " + spectrumMd.date_time.toString("yyyy.MM.dd hh:mm:ss.zzz"));
    ui->labelExperimentName->setText(spectrumMd.experiment_name);
    ui->labelAltitude->setText(QString::number(spectrumMd.location.altitude));
    ui->labelCaptureLevel->setText(db_json::CAPTURE_LEVEL_DESCRIPTION.at(spectrumMd.capture_level));
    ui->labelPlace->setText(spectrumMd.location.place_name);

    ui->labelSunElevation->setText(QString::number(spectrumMd.sun_elevation_angle, 'g', 2));
    ui->labelClouds->setText(QString::number(spectrumMd.weather_conditions.clouds_level));
    ui->labelHumidity->setText(QString::number(spectrumMd.air_conditions.humidity));
    ui->labelTemperature->setText(QString::number(spectrumMd.air_conditions.temperature));
    ui->labelWindIntencity->setText(QString::number(spectrumMd.weather_conditions.wind));
}

QString UasvViewWindow::getSpecFileNameSuffix(int &currSpecIndex)
{
    QString currSpectrumName;
    if(ui->pushButtonShowRfl->isChecked()){
        currSpecIndex = 0;
        currSpectrumName = "_rfl";
    }else if(ui->pushButtonShowPattBright->isChecked()){
        currSpecIndex = 1;
        currSpectrumName = "_patt";
    }else{
        currSpecIndex = 2;
        currSpectrumName = "_templ";
    }
    return currSpectrumName;
}

void UasvViewWindow::checkClassifAndSetupCBox(QString spClassification, QComboBox *settingCBox, QString textForEmpty)
{
    if(spClassification.isEmpty()){
        int defaultIndex = settingCBox->findText(textForEmpty);
        if(defaultIndex != -1)
            settingCBox->setCurrentIndex(defaultIndex);
    }else{
        int neededType = settingCBox->findText(spClassification);
        if(neededType != -1){
            settingCBox->setCurrentIndex(neededType);
        }
    }
}

void UasvViewWindow::on_pushButtonChangeInputFolder_clicked()
{
    QString checkPath;
    QDir existingDir(ui->lineEditInputPath->text());
    if(existingDir.isReadable()){
        checkPath = QFileDialog::getExistingDirectory(this, tr("Выбор папки для загрузки"),
                                                      ui->lineEditInputPath->text(),
                                                      QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    }else{
        checkPath = QFileDialog::getExistingDirectory(this, tr("Выбор папки для загрузки"),
                                                      "/home", QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    }
    QDir checkDir(checkPath);
    if(checkDir.exists() && !checkPath.isEmpty()){
        ui->lineEditInputPath->setText(checkPath);

        m_settings->setValue("Pathes/InputPath", checkPath);
        m_filesParser->setInputDirPath(checkPath);
        parseInputFolder();
    }
}

void UasvViewWindow::on_pushButtonShowRfl_clicked()
{
    if(ui->pushButtonShowRfl->isChecked()){
        double maxInSpectrum;
        QVector<double> waves = m_filesParser->getRflWaves();
        QVector<double> values = m_filesParser->getRflSpectrumValues(maxInSpectrum);
        drawSpectrum(waves, values, maxInSpectrum, db_json::BU_WAVELENGTH, db_json::SU_RFL, m_filesParser->currentSpName());
        ui->pushButtonShowPattBright->setChecked(false);
        ui->pushButtonShowTemplBright->setChecked(false);
    }else{
        ui->pushButtonShowRfl->setChecked(true);
    }
}

void UasvViewWindow::on_pushButtonShowPattBright_clicked()
{
    if(ui->pushButtonShowPattBright->isChecked()){
        double maxInSpectrum;
        QVector<double> waves = m_filesParser->getBrightPattWaves();
        QVector<double> values = m_filesParser->getBrightPattSpectrumValues(maxInSpectrum);
        drawSpectrum(waves, values, maxInSpectrum, db_json::BU_WAVELENGTH, db_json::SU_BRIGHT, m_filesParser->currentSpName());
        ui->pushButtonShowRfl->setChecked(false);
        ui->pushButtonShowTemplBright->setChecked(false);
    }else{
        ui->pushButtonShowPattBright->setChecked(true);
    }
}

void UasvViewWindow::on_pushButtonShowTemplBright_clicked()
{
    if(ui->pushButtonShowTemplBright->isChecked()){
        double maxInSpectrum;
        QVector<double> waves = m_filesParser->getBrightTemplWaves();
        QVector<double> values = m_filesParser->getBrightTemplSpectrumValues(maxInSpectrum);
        drawSpectrum(waves, values, maxInSpectrum, db_json::BU_WAVELENGTH, db_json::SU_BRIGHT, m_filesParser->currentSpName());
        ui->pushButtonShowRfl->setChecked(false);
        ui->pushButtonShowPattBright->setChecked(false);
    }else{
        ui->pushButtonShowTemplBright->setChecked(true);
    }
}

void UasvViewWindow::on_pushButtonSaveChanges_clicked()
{
    m_filesParser->saveCurrentSpectrum();
}

void UasvViewWindow::on_pushButtonSaveGraph_clicked()
{
    QString basePath = ui->lineEditInputPath->text() + "/";
    QString fileName = m_filesParser->spectraNamesList().at(ui->listViewSpectraNames->currentIndex().row());
    int currSpecIndex;
    QString currSpectrumName = getSpecFileNameSuffix(currSpecIndex);
    m_plotterWidget->customPlot()->saveJpg(basePath + fileName + currSpectrumName + ".jpg");
}

void UasvViewWindow::on_pushButtonExportAsText_clicked()
{
    /*QString basePath = ui->lineEditInputPath->text() + "/";
    QString fileName = m_filesParser->spectraNamesList().at(ui->listViewSpectraNames->currentIndex().row());
    ui->graphicsViewImage->saveImageWithFov(basePath + fileName + ".bmp");

    int currSpecIndex;
    QString currSpectrumName = getSpecFileNameSuffix(currSpecIndex);
    QString spectrumMd = db_json::getStringMdFromStruct(m_filesParser->spectrum().md);
    QString spectrumAttrs = db_json::getStringSpecAttrsFromStruct(m_filesParser->spectrum().sd.attributes.at(currSpecIndex));
    QString spectrumWV = SpectrDataSaver::getStringSpectrumFromVectors(m_plotterWidget->showingBands(),
                                                                      m_plotterWidget->showingValues());*/
    //qDebug()<<"spectrum_data: "<<spectrumWV;
    /*SpectrDataSaver textSaver;
    connect(&textSaver, SIGNAL(sendTextMessage(QString)), ui->statusbar, SLOT(showMessage(QString)));
    textSaver.setSingleFileName(basePath + fileName + currSpectrumName + ".txt");
    textSaver.saveStrInSeparateThread(spectrumMd + spectrumAttrs + spectrumWV);*/
}

void UasvViewWindow::on_pushButtonSetWaveRange_clicked()
{
    double startShowingWave = m_settings->value("Settings/ShowingStartWave").toDouble();
    double endShowingWave = m_settings->value("Settings/ShowingEndWave").toDouble();
    WavesRangeDialog wavesDialog(this, m_plotterWidget->bands(), m_plotterWidget->values(), m_plotterWidget->currentMaxValue(),
                                 m_plotterWidget->bandUnits(), m_plotterWidget->getSpectrumUnits(),
                                 startShowingWave, endShowingWave);
    int ret = wavesDialog.exec();
    if(ret == 1){
        m_plotterWidget->setBandsRange(wavesDialog.minWave(), wavesDialog.maxWave());
        m_settings->setValue("Settings/ShowingStartWave", wavesDialog.minWave());
        m_settings->setValue("Settings/ShowingEndWave", wavesDialog.maxWave());
    }
}

void UasvViewWindow::on_pushButtonPrevious_clicked()
{
    if(ui->listViewSpectraNames->currentIndex().row() -  1 >= 0){
        ui->listViewSpectraNames->setCurrentIndex(m_slModel->index(ui->listViewSpectraNames->currentIndex().row() - 1, 0));
        m_filesParser->setCurrentSpectrumIndex(ui->listViewSpectraNames->currentIndex().row());
        updateDataInGui();
    }else{        

        QMessageBox::information(this, tr("БЕКАС"),
                                tr("Переход к предыдущему спектру невозможен:\n"
                                   "Вы достигли начала списка!"));
    }
}

void UasvViewWindow::on_pushButtonNext_clicked()
{
    if(ui->listViewSpectraNames->currentIndex().row() + 1 < m_slModel->rowCount()){
        ui->listViewSpectraNames->setCurrentIndex(m_slModel->index(ui->listViewSpectraNames->currentIndex().row() + 1, 0));
        m_filesParser->setCurrentSpectrumIndex(ui->listViewSpectraNames->currentIndex().row());
        updateDataInGui();
    }else{

        QMessageBox::information(this, tr("БЕКАС"),
                                  tr("Переход к следующему спектру невозможен:\n"
                                     "Вы достигли конца списка!"));
    }
}

void UasvViewWindow::on_listViewSpectraNames_activated(const QModelIndex &index)
{
    m_filesParser->setCurrentSpectrumIndex(index.row());
    updateDataInGui();
}

void UasvViewWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){

    case Qt::Key_Left:
        on_pushButtonPrevious_clicked();
        break;

    case Qt::Key_Right:
        on_pushButtonNext_clicked();
        break;

    case Qt::Key_Control:
    case Qt::Key_Shift:
        m_plotterWidget->keyPressEvent(event);

    default:
        break;

    }
}

void UasvViewWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()){
    case Qt::Key_Control:
    case Qt::Key_Shift:
        m_plotterWidget->keyReleaseEvent(event);

    default:
        break;

    }
}

void UasvViewWindow::on_pushButtonOpenSavingFolder_clicked()
{
    QDir dir;
    if(!m_filesParser->inputDirPath().isEmpty()){
        dir.setPath(m_filesParser->inputDirPath());
    }else{
        dir.setPath(m_settings->value("Pathes/InputPath").toString());
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}
