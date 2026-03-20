# -*- coding: utf-8 -*-
import numpy as np
import scipy.optimize
import time
import json
import catlib

#test data example: satellite name: _bka, sun zenith angle: 41.3, dark_pixel = [39.535587, 25.645323, 11.881793, 4.310712]

#constants
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

NUMBER_OF_WAVELENGTH = 601

def load_S_lambda_lists():
    lists = []
    for i in range(len(dark_pixel)):
        lines = [float(j[satellite_name][i]) for j in sdb]
        lists.append(lines)
    for l in lists:
        assert len(l) == NUMBER_OF_WAVELENGTH
    return lists

def compute_divider_list(S_lambda_lists):
    divider_list = []
    for lambda_list in S_lambda_lists:
        sum = 0.0
        for lambda_ in lambda_list:
            sum += lambda_
        divider_list.append(sum)
    return divider_list

def print_welcome():
    print(bcolors.OKCYAN+
          bcolors.BOLD+
          "\n*********************************\n*     cat script is running     *\n*********************************"+bcolors.ENDC)
    print ("Available satellite names: _bka _landsat8 _landsat9 _sentinel _sentinel2a-10m _sentinel2a-20m _sentinel2b-10m _sentinel2b-20m")

def print_cos_zenith_angle():
    print(bcolors.WARNING+"\nКосинус зенитного угла Солнца: \n"+bcolors.ENDC, mu_0)
    
def printFilesReadingInfo():
    print(bcolors.WARNING+'\nLoaded files information:'+bcolors.ENDC)
    print('Lambdas loaded:       {0}'.format(len(lambda_list)))
    print('T_O2 loaded:          {0}'.format(len(T_O2_list)))
    print('T_O3 loaded:          {0}'.format(len(T_O3_list)))
    print('T_H2O loaded:         {0}'.format(len(T_H2O_list)))
    print('B_lambda_teta loaded: {0}'.format(len(B_lambda_teta_list)))
    print('S_lambda loaded:      {0}x{1}'.format(
        len(S_lambda_lists), len(S_lambda_lists[0])))
    print('Dividers:             {0}'.format(divider_list))

def printResults():
    print(bcolors.WARNING + "\nРезультаты расчетов: "+ bcolors.ENDC)
    print('tau_0_a = ', round(x.x[0], 4),
          '\nbeta = ',  round(x.x[1], 4),
          '\ng = ',     round(x.x[2], 4),
          '\nalb = ',   round(x.x[3], 4))
    print(bcolors.WARNING +'\nОшибки расчетов: '+ bcolors.ENDC, x.fun)
    print(bcolors.WARNING +"Время выполнения: " + bcolors.ENDC,
          (time.time() - start_time))
    print("\n")

def assertListsSizes():
    assert NUMBER_OF_WAVELENGTH == len(T_O2_list)
    assert NUMBER_OF_WAVELENGTH == len(T_O3_list)
    assert NUMBER_OF_WAVELENGTH == len(T_H2O_list)
    assert NUMBER_OF_WAVELENGTH == len(S_lambda_lists[0])
    assert NUMBER_OF_WAVELENGTH == len(B_lambda_teta_list)

# Calculate errors with C++ catlib
def equasion(var):
    tau_0_a, beta, g, alb = var        
    EQ = catlib.compute_EQ(
                      B_lambda_teta_list,
                      T_O2_list,
                      T_O3_list,
                      T_H2O_list,
                      S_lambda_lists,
                      mu_0,
                      alb,
                      tau_0_a,
                      beta,
                      g,
                      tau_m,
                      lambda_list,
                      divider_list,
                      dark_pixel
                      )
    return EQ

# Entry point to main function
if __name__ == '__main__':
    # start script time point
    start_time = time.time()
    
    #welcome message
    print_welcome()
    satellite_name = input("enter satellite name: ")

    # sun zenith angle
    angle = float(input("enter sun zenith angle: "))
    mu_0 = np.cos(np.deg2rad(angle))
    print_cos_zenith_angle()

    # dark pixel test data
    dark_pixel = [39.535587, 25.645323, 11.881793, 4.310712]

    # load data from sdb.json file
    sdb = json.load(open('sdb.json'))
    lambda_list        = [float(i['wavelength']) for i in sdb]
    S_lambda_lists     = load_S_lambda_lists()
    divider_list       = compute_divider_list(S_lambda_lists)
    T_O2_list          = [float(i['o2']) for i in sdb]
    T_O3_list          = [float(i['o3']) for i in sdb]
    T_H2O_list         = [float(i['h2o']) for i in sdb]
    B_lambda_teta_list = [float(i['sun']) for i in sdb]

    # check lists sizes
    assertListsSizes()

    # print information about sizes of loaded data from files
    printFilesReadingInfo()

    # Calculate tau_m only one time at the start
    tau_m = catlib.compute_tau_m(lambda_list)

    # Least squares solver
    x = scipy.optimize.least_squares(equasion, np.asarray(
        [0.1, 2, 0.1, 0.01]), bounds=([0, 0.1, 0.1, 0.001], [1, 4, 1, 0.5]))

    # print final results
    printResults()
    
