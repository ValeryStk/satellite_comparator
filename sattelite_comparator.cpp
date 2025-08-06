#include "sattelite_comparator.h"
#include "ui_sattelite_comparator.h"
#include "qcustomplot.h"
#include "json_utils.h"
#include "message_reporter.h"
#include <QPen>

namespace{

bool areVectorsEqual(const QVector<double> &a,
                     const QVector<double> &b)
{
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin());
}

}


SatteliteComparator::SatteliteComparator(QVector<double> device_waves,
                                         QVector<double> device_values,
                                         QVector<double> satellite_waves,
                                         QVector<double> satellite_values):
    ui(new Ui::SatteliteComparator),
    m_sdb(get_sdb()),
    m_all_satellites_data(get_satellites_data()),
    m_common_wave_grid(get_common_waves())

{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Окно сопоставления спектральных данных со спутниковыми");
    m_plot =  ui->widget_plot_comparator;
    m_plot->xAxis->setLabel("длина волны, нм");
    m_plot->legend->setVisible(true);
    initial_fill_data_to_show(device_waves,
                              device_values,
                              satellite_waves,
                              satellite_values);
    /*if(auto_detect_satellite()==false){
        uts::showErrorMessage("Ошибка загрузки данных","Данные для спутника не найдены.");
        this->close();
    };*/
    addCharts();


    //fold_spectr_to_satellite_responses();

    //check_intersection({400,500,600,700,900},{390,401,402,403,505,1000});
    //show();

}

SatteliteComparator::~SatteliteComparator()
{
    delete ui;
}

void SatteliteComparator::addCharts()
{
    for(int i=0;i<(int)plot_type::PLOT_TYPE_SIZE;++i){
        QCPGraph *graph = m_plot->addGraph();
        plot_type pt = (plot_type)i;
        QPen pen;
        switch(pt){
        case plot_type::ORIGIN_DEVICE_SPECTR:
            pen.setWidth(2);
            pen.setColor(Qt::red);
            m_plot->graph(i)->setPen(pen);
            m_plot->graph(i)->setName("Спектр ГСС");
            break;
        case plot_type::INTERPOLATED_DEVICE_SPECTR:
            graph->setLineStyle(QCPGraph::lsLine);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));
            pen.setWidth(2);
            pen.setColor(Qt::green);
            m_plot->graph((int)pt)->setPen(pen);
            m_plot->graph(i)->setName("Свёрнутый спектр ГСС");
            break;
        case plot_type::SATELLITE_SPECTR:
            graph->setLineStyle(QCPGraph::lsLine);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 10));
            pen.setWidth(2);
            pen.setColor(Qt::blue);
            m_plot->graph((int)pt)->setPen(pen);
            m_plot->graph(i)->setName(m_sat_data.alias);
            break;
        case plot_type::PLOT_TYPE_SIZE:
            break;
        }
    }
}

void SatteliteComparator::base_check_before_interpolation(const QVector<double>& x,
                                                          const QVector<double>& y)
{
    if (x.size() != y.size() || x.empty()) {
        //throw std::exception("Sizes must be the same");
    }

    // Проверка, что вектор x отсортирован по возрастанию
    if (!std::is_sorted(x.begin(), x.end())) {
        //throw std::exception("Вектор x должен быть отсортирован по возрастанию.");
    }
}


// Возвращает интерполированное значение для нового x
double SatteliteComparator::linearInterpolation(const QVector<double>& x,
                                                const QVector<double>& y,
                                                const double target_x)
{

    if (target_x < x.front() || target_x > x.back()) {
        //throw std::exception("Целевая точка вне диапазона.");
        return -1;
    }

    // Проверка, существует ли длина волны в массиве
    auto it = std::find(x.begin(), x.end(), target_x);
    if (it != x.end()) {
        int idx = static_cast<int>(std::distance(x.begin(), it));
        return y[idx]; // Возврат значения без интерполяции
    }

    // Найти место для вставки, чтобы сохранить упорядоченность
    auto lower = std::lower_bound(x.begin(), x.end(), target_x);
    if (lower == x.begin()) {
        return y.front(); // Если target равен первому элементу
    }

    // Индексы для интерполяции
    int idx = static_cast<int>(std::distance(x.begin(), lower));
    double x1 = x[idx - 1], x2 = x[idx];
    double y1 = y[idx - 1], y2 = y[idx];

    // Линейная интерполяция
    return y1 + (target_x - x1) * (y2 - y1) / (x2 - x1);
}

QHash<QString, satellites_data> SatteliteComparator::get_satellites_data()
{

    satellites_data sd;
    QJsonObject json_satellites = m_sdb["satellites"].toObject();
    m_satellites_list = json_satellites.keys();
    //qDebug()<<"sat_list: "<<m_satellites_list;
    QHash<QString,satellites_data> all_responses;
    for(auto &sk:m_satellites_list){
        auto obj = json_satellites[sk].toObject();
        sd.responses = jsn::getMatrixFromJsonArray(obj["responses"].toArray());
        sd.alias = obj["alias"].toString();
        sd.bands = jsn::getMatrixFromJsonArray(obj["bands"].toArray());
        sd.central_waves = jsn::getVectorDoubleFromJsonArray(obj["central_waves"].toArray());
        all_responses.insert(sk,sd);
    }
    return all_responses;
}

QPair<QVector<double>,QVector<double>> SatteliteComparator::interpolate(
        const QVector<double>& x,
        const QVector<double>& y,
        const QVector<double>& new_x)
{
    base_check_before_interpolation(x,y);
    QVector<double> new_values;
    QVector<double> existed_waves;
    for(int i=0;i<new_x.size();++i){
        if(-1==linearInterpolation(x,y,new_x[i]))continue;
        new_values.push_back(linearInterpolation(x,y,new_x[i]));
        existed_waves.push_back(new_x[i]);
    }
    return {existed_waves,new_values};//{existed_waves,new_values};
}

void SatteliteComparator::replot()
{
    m_plot->graph((int)plot_type::ORIGIN_DEVICE_SPECTR)->setData(m_data_to_show.device_waves,
                                                                 m_data_to_show.device_values);
    m_plot->graph((int)plot_type::SATELLITE_SPECTR)->setData(m_data_to_show.satellite_waves,
                                                             m_data_to_show.satellite_values);
    m_plot->graph((int)plot_type::INTERPOLATED_DEVICE_SPECTR)->setData(m_data_to_show.satellite_waves,
                                                                       m_data_to_show.folded_device_values);
    m_plot->rescaleAxes();
    m_plot->replot();
}

int SatteliteComparator::tryToFindTheSameVector(const QVector<QVector<double>> &vectorOfVectors,
                                                const QVector<double> &targetVector)
{
    for (int i = 0; i < vectorOfVectors.size(); ++i) {
        if (areVectorsEqual(vectorOfVectors[i], targetVector)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

QJsonObject SatteliteComparator::get_sdb()
{
    QJsonObject sdb;
    jsn::getJsonObjectFromFile(":/res/sd.json",sdb);
    return sdb;
}

QVector<double> SatteliteComparator::get_common_waves()
{
    return jsn::getVectorDoubleFromJsonArray(m_sdb["_common_wave_grid"].toArray());
}

QVector<QVector<double>> SatteliteComparator::get_bands(const QString& satellite_name)
{
    auto satellite = m_sdb["satellites"].toObject().value(satellite_name).toObject();
    auto bands = satellite["bands"].toArray();
    return jsn::getMatrixFromJsonArray(bands);
}

void SatteliteComparator::initial_fill_data_to_show(const QVector<double>& device_waves,
                                                    const QVector<double>& device_values,
                                                    const QVector<double>& satellite_waves,
                                                    const QVector<double>& satellite_values)
{
    m_data_to_show.device_waves = device_waves;
    m_data_to_show.device_values = device_values;
    m_data_to_show.satellite_waves = satellite_waves;
    m_data_to_show.satellite_values = satellite_values;
}

bool SatteliteComparator::set_satellite_responses(const QString& satellite_name)
{
    for(const auto &sat_name:qAsConst(m_satellites_list)){
        auto check = m_all_satellites_data[sat_name];
        //qDebug()<<"--->"<<m_data_to_show.satellite_waves<<"----"<<check.central_waves;
        if(sat_name==satellite_name){
            m_sat_data.satellite_name = sat_name;
            m_sat_data.alias = check.alias;
            m_sat_data.bands = check.bands;
            m_sat_data.responses = check.responses;
            m_sat_data.central_waves = check.central_waves;
            qDebug()<<"AUTO_DETECTED_SATELLITE_NAME: "<<sat_name;
            return true;
        };
    }
    return false;
}

void SatteliteComparator::compare_spectrs()
{
    auto x_y = interpolate(m_data_to_show.device_waves,
                           m_data_to_show.device_values,
                           m_common_wave_grid);

    //dv::show(x_y.first,x_y.second);
}

QVector<double> SatteliteComparator::fold_spectr_to_satellite_responses()
{
    static const int SATTELITE_WAVE_OFFSET = 400;
    QVector<double>folded_spectr(m_sat_data.bands.size());
    QVector<double>satellite_bands_sum(m_sat_data.bands.size());
    QVector<double>device_spectr_bands_sum(m_sat_data.bands.size());
    auto x_y = interpolate(m_data_to_show.device_waves,
                           m_data_to_show.device_values,
                           m_common_wave_grid);



    for(int i=0;i<m_sat_data.bands.size();++i){
        int start = m_sat_data.bands[i][0]-SATTELITE_WAVE_OFFSET;
        int end   = m_sat_data.bands[i][1]-SATTELITE_WAVE_OFFSET;
        for(int j=start;j<end;++j){
            //qDebug()<<"---resp-->"<<i<<m_sat_data.responses[j][i]<<"\n";
            satellite_bands_sum[i]+=m_sat_data.responses[j][i];
            device_spectr_bands_sum[i]+=x_y.second[j]*m_sat_data.responses[j][i];
        }
    }
    qDebug()<<"bands_sum-->"<<satellite_bands_sum;
    qDebug()<<"device_spectr_bands_sum-->"<<device_spectr_bands_sum;

    for(int i=0;i<device_spectr_bands_sum.size();++i){
        folded_spectr[i]=device_spectr_bands_sum[i]/satellite_bands_sum[i];
    }
    qDebug()<<"folded_spectr"<<folded_spectr;


    return folded_spectr;
    //dv::show(m_data_to_show.device_waves,m_data_to_show.device_values);
    /*m_plot->graph((int)plot_type::SATELLITE_SPECTR)->setData(m_data_to_show.satellite_waves,
                                                             m_data_to_show.satellite_values);
    m_plot->graph((int)plot_type::ORIGIN_DEVICE_SPECTR)->setData(m_data_to_show.device_waves,
                                                                 m_data_to_show.device_values);

    m_plot->graph((int)plot_type::INTERPOLATED_DEVICE_SPECTR)->setData(m_data_to_show.satellite_waves,
                                                                       folded_spectr);
    m_plot->rescaleAxes();
    m_plot->replot();*/
}


QVector<double> SatteliteComparator::check_intersection(
        const QVector<double> &waves_1,
        const QVector<double> &waves_2)
{
    QVector<double>intersection;
    double start = std::max(waves_1.first(), waves_2.first());
    double end = std::min(waves_1.last(), waves_2.last());
    qDebug()<<"start-end-->"<<start<<end;
    if (start <= end){
        intersection = {start,end};
        qDebug()<<"***INTERSECTION***--->"<<intersection;
        return {start, end}; // Возвращаем пересечение
    }
    else
        return {}; // Нет пересечения
}


