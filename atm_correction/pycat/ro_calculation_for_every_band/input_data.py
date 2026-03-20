def load_T_O2_list():
    lines = open('files/o2.txt').readlines()
    return [float(l) for l in lines]


def load_T_O3_list():
    lines = open('files/o3.txt').readlines()
    return [float(l) for l in lines]


def load_T_H2O_list():
    lines = open('files/h2o.txt').readlines()
    return [float(l) for l in lines]


def load_lambda_list():
    lines = open('files/lmb.txt').readlines()
    return [float(l) for l in lines]


def load_B_lambda_teta_list():
    lines = open('files/sun.txt').readlines()
    return [float(l) for l in lines]


def compute_divider_list(S_lambda_lists):
    divider_list = []
    for lambda_list in S_lambda_lists:
        sum = 0.0
        for lambda_ in lambda_list:
            sum += lambda_
        divider_list.append(sum)
    return divider_list





# def find_nearest(array, value):
#     array = np.asarray(array)
#     return array

def input_dark_pixel():
    pixel = list(map(float, input('Введите значения темного пикселя через пробел ').split()))
    # lines = open('files/darkpix-2020-05-01_4.txt').readlines()
    return pixel