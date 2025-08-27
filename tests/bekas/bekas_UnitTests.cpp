#include "bekas_UnitTests.h"

#include "GuiModules/UasvViewWindow.h"
#include <QDebug>


namespace{


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
  UasvViewWindow *bekas_window = new UasvViewWindow;
  bekas_window->show();
  QTest::qWait(300000);
  app.exec();
}


QTEST_MAIN(bekas_UnitTests)
