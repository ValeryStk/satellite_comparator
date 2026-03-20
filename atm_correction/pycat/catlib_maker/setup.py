import pybind11
from distutils.core import setup, Extension

ext_modules = [
    Extension(
        'catlib',
        ['cat_lib.cpp'],
        include_dirs=[pybind11.get_include()],
        language='c++',
        extra_compile_args=['-std=c++11'],
    ),
]

setup(
    name='catlib',
    version='0.0.1',
    author='ValeryStk',
    author_email='tor-tech@mail.ru',
    description='pybind11 extension',
    ext_modules=ext_modules,
    requires=['pybind11']
)