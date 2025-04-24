#ifndef DYNAMIC_CHECKBOX_WIDGET_H
#define DYNAMIC_CHECKBOX_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QString>
#include <QList>

#define RGB_NUMBER 3

#define RED 0
#define GREEN 1
#define BLUE 2

constexpr char RES_COLOR_RED[] = ":/res/red.svg";
constexpr char RES_COLOR_GREEN[] = ":/res/green.svg";
constexpr char RES_COLOR_BLUE[] = ":/res/blue.svg";
constexpr char RES_COLOR_BLACK[] = ":/res/black.svg";

class DynamicCheckboxWidget : public QWidget {
    Q_OBJECT
public:
    explicit DynamicCheckboxWidget(const QList<QString>& labels,
                                   QVBoxLayout *layout = nullptr);

QVector<QPair<int,int>> get_choosed_bands();

void setInitialCheckBoxesToggled(const QVector<int>& toToggle);

private slots:
    void onCheckboxStateChanged(QCheckBox *checkBox);

signals:
    void choosed_bands_changed();

private:
    QList<QCheckBox*> checkboxes;
    QList<QPair<QCheckBox*,int>> checkedOrder; // Список для отслеживания порядка выбора чекбоксов
    const int MAX_POSSIBLE_CHECKED = RGB_NUMBER;
    bool used_colors[RGB_NUMBER] = {false,false,false};//RGB
    QString colors_svg[RGB_NUMBER] = {RES_COLOR_BLUE,RES_COLOR_GREEN,RES_COLOR_RED};
    int color_sequence[RGB_NUMBER] = {BLUE,GREEN,RED};
};

#endif // DYNAMIC_CHECKBOX_WIDGET_H
