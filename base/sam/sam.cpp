#include "sam.h"
#include <cmath>

namespace sam {

    void computeFFT(const double* input, double* output, int size) {
        // Заглушка: здесь будет FFT
        for (int i = 0; i < size; ++i)
            output[i] = input[i]; // временно просто копируем
    }

    double powerSpectrum(const double* signal, int size) {
        double sum = 0.0;
        for (int i = 0; i < size; ++i)
            sum += signal[i] * signal[i];
        return sum / size;
    }
}
