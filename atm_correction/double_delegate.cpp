#include <QDoubleValidator>
#include <QLineEdit>
#include <QStyledItemDelegate>

class DoubleDelegate : public QStyledItemDelegate {
public:
    explicit DoubleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    // Этот метод создает поле ввода, когда пользователь кликает на ячейку
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setAlignment(Qt::AlignCenter);
        // Настраиваем валидатор для вещественных чисел
        QDoubleValidator *validator = new QDoubleValidator(editor);
        validator->setNotation(
            QDoubleValidator::StandardNotation);  // Стандартная запись
                                                  // (например, 123.45)
        validator->setLocale(
            QLocale::C);  // Точка в качестве разделителя (1.23 вместо 1,23)

        editor->setValidator(validator);
        return editor;
    }

    // 2. Выравнивание обычного текста ячейки (когда редактирование завершено)
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        // Задаем выравнивание по центру по горизонтали и вертикали
        option->displayAlignment = Qt::AlignCenter;
    }
};
