#ifndef SAM_H
#define SAM_H

namespace sam {

    // Преобразование Фурье
    void computeFFT(const double* input, double* output, int size);

    // Спектральная плотность мощности
    double powerSpectrum(const double* signal, int size);
}

#endif // SAM_H
