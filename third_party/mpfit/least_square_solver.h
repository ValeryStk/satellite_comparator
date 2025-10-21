#pragma once
#include <vector>
#include <functional>

class LeastSquareSolver {
public:
    using ModelFunction = std::function<double(double, const std::vector<double>&)>;

    LeastSquareSolver();
    void setModel(ModelFunction model, int numParams);
    void setData(const std::vector<double>& x, const std::vector<double>& y);
    void setInitialGuess(const std::vector<double>& guess);
    bool solve();
    std::vector<double> getParameters() const;

private:
    ModelFunction modelFunc;
    int paramCount;
    std::vector<double> xData, yData;
    std::vector<double> parameters;

    // Статический доступ к текущему экземпляру
    static LeastSquareSolver* activeSolver;

    friend int mpfit_model(int m, int n, double* p, double* dy, double** dvec, void* vars);
};

