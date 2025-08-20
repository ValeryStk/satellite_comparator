#include "sattelite_comparator.h"



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
    m_sdb(get_sdb()),
    m_all_satellites_data(get_satellites_data()),
    m_common_wave_grid(get_common_waves())

{
initial_fill_data_to_show(device_waves,
                          device_values,
                          satellite_waves,
                          satellite_values);
}

SatteliteComparator::~SatteliteComparator()
{

}

BASE_CHECK_RESULT SatteliteComparator::base_check_before_interpolation(
        const QVector<double>& waves1,
        const QVector<double>& waves2)
{
    if(waves1.empty()) return BASE_CHECK_RESULT::WAVES_IS_EMPTY;

    if(waves2.empty()) return BASE_CHECK_RESULT::VALUES_IS_EMPTY;

    if(waves1.size()!=waves2.size()) return BASE_CHECK_RESULT::SIZES_ARE_NOT_THE_SAME;

    if (!std::is_sorted(waves1.begin(), waves1.end())) {
        return BASE_CHECK_RESULT::WAVES_IS_NOT_SORTED;
    }
    if(check_intersection(waves1, waves2).empty()){
        return BASE_CHECK_RESULT::NO_INTERSECTION;
    }
    return BASE_CHECK_RESULT::OK;
}


// Возвращает интерполированное значение для нового x
double SatteliteComparator::linearInterpolation(const QVector<double>& x,
                                                const QVector<double>& y,
                                                const double target_x)
{

    if (target_x < x.front() || target_x > x.back()) {
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
        const QVector<double>& new_x,
        BASE_CHECK_RESULT& result_status)
{
    QVector<double> new_values;
    QVector<double> existed_waves;
    result_status = base_check_before_interpolation(x,y);

    if( result_status == BASE_CHECK_RESULT::OK){
        for(int i=0;i<new_x.size();++i){
            if(-1==linearInterpolation(x,y,new_x[i]))continue;
            new_values.push_back(linearInterpolation(x,y,new_x[i]));
            existed_waves.push_back(new_x[i]);
        }
        return {existed_waves,new_values};
    }
    return {};
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

}

bool SatteliteComparator::set_satellite_responses(const QString& satellite_name)
{
    for(const auto &sat_name:qAsConst(m_satellites_list)){
        auto check = m_all_satellites_data[sat_name];
        if(sat_name==satellite_name){
            m_sat_data.satellite_name = sat_name;
            m_sat_data.alias = check.alias;
            m_sat_data.bands = check.bands;
            m_sat_data.responses = check.responses;
            m_sat_data.central_waves = check.central_waves;
            return true;
        };
    }
    return false;
}

void SatteliteComparator::compare_spectrs()
{
    BASE_CHECK_RESULT result;
    auto x_y = interpolate(m_comparator_data.device_waves,
                           m_comparator_data.device_values,
                           m_common_wave_grid,
                           result);

}

QVector<double> SatteliteComparator::fold_spectr_to_satellite_responses()
{
    static const int SATTELITE_WAVE_OFFSET = 400;
    QVector<double>folded_spectr(m_sat_data.bands.size());
    QVector<double>satellite_bands_sum(m_sat_data.bands.size());
    QVector<double>device_spectr_bands_sum(m_sat_data.bands.size());
    BASE_CHECK_RESULT status;
    auto x_y = interpolate(m_comparator_data.device_waves,
                           m_comparator_data.device_values,
                           m_common_wave_grid,
                           status);

    for(int i=0;i<m_sat_data.bands.size();++i){
        int start = m_sat_data.bands[i][0]-SATTELITE_WAVE_OFFSET;
        int end   = m_sat_data.bands[i][1]-SATTELITE_WAVE_OFFSET;
        for(int j=start;j<end;++j){
            satellite_bands_sum[i]+=m_sat_data.responses[j][i];
            device_spectr_bands_sum[i]+=x_y.second[j]*m_sat_data.responses[j][i];
        }
    }
    for(int i=0;i<device_spectr_bands_sum.size();++i){
        folded_spectr[i]=device_spectr_bands_sum[i]/satellite_bands_sum[i];
    }
    return folded_spectr;
}


QVector<double> SatteliteComparator::check_intersection(
        const QVector<double> &waves_1,
        const QVector<double> &waves_2)
{
    QVector<double>intersection;
    double start = std::max(waves_1.first(), waves_2.first());
    double end = std::min(waves_1.last(), waves_2.last());
    if (start <= end){
        intersection = {start,end};
        return {start, end};
    }
    else
        return {};
}


