import numpy as np
import scipy
from osgeo import gdal
from math import pi
from tqdm import tqdm

import input_data
import functions


def load_S_lambda_lists():
    lists = []
    for i in range(len(dark_pixel)):
        file_name = 'files/i{0}.txt'.format(i + 1)
        lines = open(file_name).readlines()

        list_ = []
        for l in lines:
            not_added = True
            while not_added:
                try:
                    list_.append(float(l))
                    not_added = False
                except:
                    l = l[1:]
        lists.append(list_)
        # lists.append([float(l) for l in lines])
    for l in lists:
        assert len(l) == len(lists[0])
        #print(lists)
    return lists


def equasion(var):
    tau_0_a, beta, g, alb = var
    eq = []
    for i in range(0, len(dark_pixel)):
        B1 = functions.compute_B1(T_O2_list, T_O3_list, T_H2O_list, lambda_list, S_lambda_lists[i], B_lambda_teta_list, mu_0,
                                  tau_m_0, lambda_0, p, tau_0_a, beta, g, q, tau_e)
        B2 = functions.compute_B2(lambda_list, S_lambda_lists[i], B_lambda_teta_list, T_O2_list, T_O3_list, T_H2O_list, mu_0,
                                  tau_m_0, lambda_0, alb, tau_0_a, beta, g, tau_e) * alb / pi
        B = (B1 + B2) / divider_list[i]
        eq.append(dark_pixel[i] - B)
    return eq


def equasion_for_ro(ro):
    B2 = functions.compute_B2(lambda_list, Band_S, B_lambda_teta_list, T_O2_list, T_O3_list, T_H2O_list, mu_0, tau_m_0, lambda_0,
                              ro, tau_0_a, beta, g, tau_e) * ro / pi
    a = (B1 + B2) / divider

    return (B - a)

# Press the green button in the gutter to run the script.
if __name__ == '__main__':

    #angle = float(input('Введите зенитный угол Солнца из метаданных '))
    angle = 24.0517
    mu_0 = np.cos(np.deg2rad(angle))

    q = 1
    dark_pixel = [56.762463, 39.694557, 20.714876, 10.796495]
    print("Косинус зенитного угла Солнца равен ", mu_0)
    tau_m_0 = 0.101
    lambda_0 = 0.55
    p = 1.25
    tau_e = 0.04

    #dark_pixel = input_data.input_dark_pixel()

    print('Loading data:')
    lambda_list = input_data.load_lambda_list()
    print('Lambdas loaded: {0}'.format(len(lambda_list)))

    T_O2_list = input_data.load_T_O2_list()
    print('T_O2 loaded: {0}'.format(len(T_O2_list)))

    T_O3_list = input_data.load_T_O3_list()
    print('T_O3 loaded: {0}'.format(len(T_O3_list)))

    T_H2O_list = input_data.load_T_H2O_list()
    print('T_H2O loaded: {0}'.format(len(T_H2O_list)))

    S_lambda_lists = load_S_lambda_lists()
    print('S_lambda loaded: {0}x{1}'.format(len(S_lambda_lists), len(S_lambda_lists[0])))

    divider_list = input_data.compute_divider_list(S_lambda_lists)
    print('Dividers: {0}'.format(divider_list))

    B_lambda_teta_list = input_data.load_B_lambda_teta_list()
    print('B_lambda_teta loaded: {0}'.format(len(B_lambda_teta_list)))

    assert len(lambda_list) == len(T_O2_list)
    assert len(lambda_list) == len(T_O3_list)
    assert len(lambda_list) == len(T_H2O_list)
    assert len(lambda_list) == len(S_lambda_lists[0])
    assert len(lambda_list) == len(B_lambda_teta_list)

    print('All data loaded.')
    print('----------------')
    print(isinstance(lambda_list, list))
    # , loss='soft_l1' , f_scale=0.1
    x = scipy.optimize.least_squares(equasion, np.asarray([0.1, 2, 0.1, 0.01]),
                                     bounds=([0, 0.1, 0.1, 0.01], [1, 4, 1, 0.5]))
    print(x)
    print("Результаты расчеты следующие:")
    print('tau_0_a = ', round(x.x[0], 2), '\nbeta = ', round(x.x[1], 2), '\ng = ', round(x.x[2], 2), '\nalb = ',
          round(x.x[3], 2))
    # mistakes = []
    # for i in range(len(dark_pixel)):
    #     mistakes.append(x.fun[i] * 100 / dark_pixel[i])
    print('Ошибка расчетов составляет: ', x.fun)
    # np.savetxt('result.txt', result)

    tau_0_a = round(x.x[0], 2)
    beta = round(x.x[1], 2)
    g = round(x.x[2], 2)

    image = gdal.Open('lacrau-20220618-radiance.tif', gdal.GA_ReadOnly)
    projection = image.GetProjection()
    transform = image.GetGeoTransform()

    for b in tqdm(range(1, image.RasterCount + 1)):

        print('Канал ' + str(b) + ' загружен')
        band = image.GetRasterBand(b)
        arr = band.ReadAsArray()
        # arr = arr * 10000
        print(arr)
        arr_corr = np.empty_like(arr, dtype=float)
        [cols, rows] = arr.shape

        pixels = np.asarray(arr)
        minimum = np.amin(arr)
        maximum = np.amax(arr)
        rad = functions.generate_range(int(minimum) + 1, int(maximum), 100)
        # print(rad)
        refl = np.empty_like(rad)
        divider = divider_list[b - 1]
        B1 = functions.compute_B1(T_O2_list, T_O3_list, T_H2O_list, lambda_list, S_lambda_lists[b - 1], B_lambda_teta_list, mu_0,
                                  tau_m_0, lambda_0, p, tau_0_a, beta, g, q, tau_e)
        # print(B1)
        Band_S = S_lambda_lists[b - 1]
        for i in range(len(rad)):
            B = rad[i]
            # root = scipy.optimize.fsolve(equasion_for_ro, x0=0.1)
            root = scipy.optimize.least_squares(equasion_for_ro, 0.1, bounds=(0.003, 1)).x
            refl[i] = root
            # print(refl[i])
        # print(refl)
        print('LUT для канала ' + str(b) + ' посчитана')
        reflectance = np.empty_like(pixels)
        print('Приступаю к расчету КСЯ')
        for i in range(len(arr)):
            # print(i)
            for j in range(len(arr[i])):
                idx = (np.abs(arr[i][j] - rad)).argmin()
                reflectance[i][j] = refl[idx]

        driver = gdal.GetDriverByName("GTiff")
        metadata = driver.GetMetadata()

        outFileName = 'lacrau-20220618-CAT-band' + str(b) + '.tif'
        # outFileName = 'lacrau-level-1с-20220811-10m-band10.tif'

        outdata = driver.Create(outFileName, rows, cols, 1, gdal.GDT_Float32)
        outdata.SetProjection(projection)
        outdata.SetGeoTransform(transform)
        outdata.GetRasterBand(1).WriteArray(reflectance)
        outdata = None


# See PyCharm help at https://www.jetbrains.com/help/pycharm/
