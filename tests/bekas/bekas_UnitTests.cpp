#include "bekas_UnitTests.h"

#include "GuiModules/UasvViewWindow.h"
#include <QDebug>


namespace{

void moveCursorAroundWindow(UasvViewWindow *window) {
    QRect rect = window->rect(); // Координаты внутри окна
    int margin = 30; // Отступ от краёв
    QPoint topLeft     = QPoint(margin, margin);
    QPoint topRight    = QPoint(rect.width() - margin, margin);
    QPoint bottomRight = QPoint(rect.width() - margin, rect.height() - margin);
    QPoint bottomLeft  = QPoint(margin, rect.height() - margin);
    QPoint center      = rect.center();

    auto moveAndWait = [](const QPoint &pos) {
        QCursor::setPos(pos);
        QThread::msleep(2000); // Пауза 2 секунды
    };

    moveAndWait(center);
    moveAndWait(topLeft);
    moveAndWait(topRight);
    moveAndWait(bottomRight);
    moveAndWait(bottomLeft);
}

} // end namespace


bekas_UnitTests::bekas_UnitTests()
{
}

void bekas_UnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void bekas_UnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void bekas_UnitTests::init()
{
    // Инициализация перед каждым тестом

}

void bekas_UnitTests::cleanup()
{
    // Очистка после каждого теста
}

void bekas_UnitTests::bekasTest()
{
    int argc = 0;
    QApplication app(argc, nullptr);
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::red); // простой красный квадрат
    QCursor customCursor(pixmap);
    QApplication::setOverrideCursor(customCursor);
    UasvViewWindow *bekas_window = new UasvViewWindow;
    bekas_window->move(0, 0);
    bekas_window->show();

    QTimer::singleShot(1000, [=]() {
        moveCursorAroundWindow(bekas_window);
    });
    app.exec();
}


QTEST_MAIN(bekas_UnitTests)
