import numpy as np
import scipy

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