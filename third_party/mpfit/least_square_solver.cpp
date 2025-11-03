#include "least_square_solver.h"
#include "mpfit.h"
#include <cstring>
#include <QDebug>

LeastSquareSolver* LeastSquareSolver::activeSolver = nullptr;

int mpfit_model(int m,
                int n,
                double* p,
                double* dy,
                double** dvec,
                void* /*vars*/) {
    auto* solver = LeastSquareSolver::activeSolver;
    //if (!solver || !dvec || !dvec[0]) return -1;

    for (int i = 0; i < m; ++i) {
        dy[i] = solver->yData[i] - (p[0]*solver->xData[i] + p[1]);
        //qDebug()<<dy[i]<<"----"<<solver->xData[i]<<"----"<<solver->yData[i]<<"-----"<<p[0]<<"------"<<p[1];
    }

    return 0;
}

LeastSquareSolver::LeastSquareSolver() : paramCount(0) {}

void LeastSquareSolver::setModel(ModelFunction model, int numParams) {
    modelFunc = model;
    paramCount = numParams;
}

void LeastSquareSolver::setData(const std::vector<double>& x, const std::vector<double>& y) {
    xData = x;
    yData = y;
}

void LeastSquareSolver::setInitialGuess(const std::vector<double>& guess) {
    parameters = guess;
}

bool LeastSquareSolver::solve() {
    std::vector<mp_par> pars(paramCount);
    for (int i = 0; i < paramCount; ++i) {
        pars[i].limited[0] = 1;
        pars[i].limited[1] = 1;
        pars[i].limits[0] = -100.0;
        pars[i].limits[1] = 100.0;
        pars[i].side = -100;
        pars[i].step = 0.01;
    }

    mp_result result;
    std::memset(&result, 0, sizeof(result));
    result.xerror = new double[paramCount];

    double* dydata[] = { yData.data() };

    activeSolver = this;

    int status = mpfit(mpfit_model,
                       static_cast<int>(xData.size()),
                       paramCount,
                       parameters.data(),
                       pars.data(),
                       nullptr,
                       dydata,
                       &result);

    activeSolver = nullptr;
    delete[] result.xerror;
    return status >= 0;
}

std::vector<double> LeastSquareSolver::getParameters() const {
    return parameters;
}

