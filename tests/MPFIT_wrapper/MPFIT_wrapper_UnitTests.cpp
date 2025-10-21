#include "MPFIT_wrapper_UnitTests.h"

#include <QDebug>
#include "least_square_solver.h"


namespace{


} // end namespace


MPFIT_wrapper_UnitTests::MPFIT_wrapper_UnitTests()
{

    LeastSquareSolver solver;
        solver.setModel([](double x, const std::vector<double>& p) {
            return p[0] * x + p[1]; // линейная модель
        }, 2);

        solver.setData({1.0, 2.6, 3.7}, {2.1, 4.1, 6.0});
        solver.setInitialGuess({1.0, 1.0});

        if (solver.solve()) {
            auto params = solver.getParameters();
            qDebug() << "Slope:" << params[0] << "Intercept:" << params[1];
        } else {
            qDebug() << "Fit failed.";
        }

}

void MPFIT_wrapper_UnitTests::initTestCase()
{
    // Инициализация перед запуском всех тестов
}

void MPFIT_wrapper_UnitTests::cleanupTestCase()
{
    // Очистка после выполнения всех тестов
}

void MPFIT_wrapper_UnitTests::init()
{
    // Инициализация перед каждым тестом

}

void MPFIT_wrapper_UnitTests::cleanup()
{
    // Очистка после каждого теста
}



QTEST_MAIN(MPFIT_wrapper_UnitTests)
