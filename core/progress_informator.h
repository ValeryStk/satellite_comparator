#ifndef PROGRESS_INFORMATOR_H
#define PROGRESS_INFORMATOR_H

#include <QLabel>

namespace msg {
extern const char kLoadLandSatTimeRowTask[];
extern const char kLoadSentinelTimeRowTask[];
extern const char kLoadSingleSentinelTask[];
extern const char kLoadSingleLandsatTask[];
extern const char kLoadSentinelWithAtmCorrection[];
extern const char kLoadSentinel_Level_1_no_atm_correction[];
extern const char kFindProcessIsRunning[];
extern const char kCalculateSen2Cor_vs_CATI_metrics[];
extern const char kFindBasePixelAsMoreLinear[];
extern const char kSaveCATIasGeoTiff[];
extern const char kSaveBaseSentinelAs20mGeoTiff[];
extern const char kChangeDetectionMetodProcessing[];
}  // namespace msg

class ProgressInformator : public QLabel {
public:
    ProgressInformator(QWidget* parent, const QString& text);
    ~ProgressInformator();
};

#endif  // PROGRESS_INFORMATOR_H
