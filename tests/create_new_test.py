import os

#шаблон файла .pro
PRO_FILE_TEMPLATE = """QT += testlib core
TARGET = {name}UnitTests
HEADERS += {name}UnitTests.h
SOURCES += {name}UnitTests.cpp
"""

#шаблон файла .h
HEADER_FILE_TEMPLATE = """#ifndef {name_upper}UNITTESTS_H
#define {name_upper}UNITTESTS_H

#include <QObject>
#include <QtTest>

class {name}UnitTests : public QObject
{{
    Q_OBJECT

public:
    {name}UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

}};

#endif // {name_upper}UNITTESTS_H
"""

#шаблон файла .cpp
CPP_FILE_TEMPLATE = """#include "{name}UnitTests.h"

#include <QDebug>


namespace{{


}} // end namespace


{name}UnitTests::{name}UnitTests()
{{
}}

void {name}UnitTests::initTestCase()
{{
    // Инициализация перед запуском всех тестов
}}

void {name}UnitTests::cleanupTestCase()
{{
    // Очистка после выполнения всех тестов
}}

void {name}UnitTests::init()
{{
    // Инициализация перед каждым тестом

}}

void {name}UnitTests::cleanup()
{{
    // Очистка после каждого теста
}}



QTEST_MAIN({name}UnitTests)
"""




# проверка на допустимое имя для класса
def test_name_checker():
    
    while True:
        
        # вводим имя нового теста
        name = input("Введите имя нового теста: ").strip()
        
        if not name:
            print("Имя не может быть пустым")
            continue
            
        if name[0].isdigit():
            print("Имя не может начинаться с цифры")
            continue
            
        if not name.replace('_', '').isalnum():
            print("Имя может содержать только буквы, цифры и подчеркивания")
            continue
        
        if any(char in 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя' for char in name.lower()):
            print("Имя не может содержать русские буквы")
            continue
              
        return name
    
TESTS_PRO="_Tests.pro"
UNITTESTS_H="UnitTests.h"
UNITTESTS_CPP="UnitTests.cpp"

def create_new_test():
    
    new_name = test_name_checker()
    
    # директория для нового теста
    dst_dir = new_name
    os.makedirs(dst_dir, exist_ok=True)
    
    # Создаем файлы из шаблонов
    with open(os.path.join(dst_dir, f"{new_name}{TESTS_PRO}"), "w", encoding="utf-8") as f:
        f.write(PRO_FILE_TEMPLATE.format(name=new_name))
    
    with open(os.path.join(dst_dir, f"{new_name}{UNITTESTS_H}"), "w", encoding="utf-8") as f:
        f.write(HEADER_FILE_TEMPLATE.format(name=new_name, name_upper=new_name.upper()))
    
    with open(os.path.join(dst_dir, f"{new_name}{UNITTESTS_CPP}"), "w", encoding="utf-8") as f:
        f.write(CPP_FILE_TEMPLATE.format(name=new_name))

    # Обновляем tests.pro
    tests_pro_path = "tests.pro"
    if os.path.exists(tests_pro_path):
        with open(tests_pro_path, "r", encoding="utf-8") as f:
            content = f.read()
        
        # Проверяем, не добавлен ли уже этот тест
        if f"SUBDIRS += {new_name}" not in content:
            with open(tests_pro_path, "a", encoding="utf-8") as f:
                f.write(f"\nSUBDIRS += {new_name}\n")
                f.write(f"{new_name}.file = {new_name}/{new_name}{TESTS_PRO}\n")
            print(f"Добавлена запись в tests.pro")
        else:
            print(f"Тест '{new_name}' уже добавлен в tests.pro")
    else:
        print("Файл 'tests.pro' не найден.")
        with open(tests_pro_path, "w", encoding="utf-8") as f:
            f.write(f"TEMPLATE = subdirs\n\nSUBDIRS += {new_name}\n")
            f.write(f"{new_name}.file = {new_name}/{new_name}{TESTS_PRO}\n")

    print(f"Тест '{new_name}' успешно создан.")
    print(f"Файлы созданы в папке: {dst_dir}/")
    print(f"   - {new_name}{TESTS_PRO}")
    print(f"   - {new_name}{UNITTESTS_H}")
    print(f"   - {new_name}{UNITTESTS_CPP}")

if __name__ == "__main__":
    create_new_test()