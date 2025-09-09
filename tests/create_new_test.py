import os

def create_new_test():
    
    # проверка на допустимое имя для класса
    while True:
        
        # вводим имя нового теста
        new_name = input("Введите имя нового теста: ").strip()
        
        if not new_name:
            print("❌ Имя не может быть пустым")
            continue
            
        if new_name[0].isdigit():
            print("❌ Имя не может начинаться с цифры")
            continue
            
        if not new_name.replace('_', '').isalnum():
            print("❌ Имя может содержать только буквы, цифры и подчеркивания")
            continue
        
        if any(char in 'абвгдеёжзийклмнопрстуфхцчшщъыьэюя' for char in new_name.lower()):
            print("❌ Имя не может содержать русские буквы")
            continue
              
        break
    
    
    # директория для нового теста
    dst_dir = new_name
    os.makedirs(dst_dir, exist_ok=True)
    
    # шаблоны для файлов
    pro_file_template = f"""QT += testlib core
TARGET = {new_name}UnitTests
HEADERS += {new_name}UnitTests.h
SOURCES += {new_name}UnitTests.cpp
"""

    header_file_template = f"""#ifndef {new_name.upper()}UNITTESTS_H
#define {new_name.upper()}UNITTESTS_H

#include <QObject>
#include <QtTest>

class {new_name}UnitTests : public QObject
{{
    Q_OBJECT

public:
    {new_name}UnitTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

}};

#endif // {new_name.upper()}UNITTESTS_H
"""

    cpp_file_template = f"""#include "{new_name}UnitTests.h"

#include <QDebug>


namespace{{


}} // end namespace


{new_name}UnitTests::{new_name}UnitTests()
{{
}}

void {new_name}UnitTests::initTestCase()
{{
    // Инициализация перед запуском всех тестов
}}

void {new_name}UnitTests::cleanupTestCase()
{{
    // Очистка после выполнения всех тестов
}}

void {new_name}UnitTests::init()
{{
    // Инициализация перед каждым тестом

}}

void {new_name}UnitTests::cleanup()
{{
    // Очистка после каждого теста
}}



QTEST_MAIN({new_name}UnitTests)
"""

    # Создаем файлы из шаблонов
    with open(os.path.join(dst_dir, f"{new_name}_Tests.pro"), "w", encoding="utf-8") as f:
        f.write(pro_file_template)
    
    with open(os.path.join(dst_dir, f"{new_name}UnitTests.h"), "w", encoding="utf-8") as f:
        f.write(header_file_template)
    
    with open(os.path.join(dst_dir, f"{new_name}UnitTests.cpp"), "w", encoding="utf-8") as f:
        f.write(cpp_file_template)

    # Обновляем tests.pro
    tests_pro_path = "tests.pro"
    if os.path.exists(tests_pro_path):
        with open(tests_pro_path, "r", encoding="utf-8") as f:
            content = f.read()
        
        # Проверяем, не добавлен ли уже этот тест
        if f"SUBDIRS += {new_name}" not in content:
            with open(tests_pro_path, "a", encoding="utf-8") as f:
                f.write(f"\nSUBDIRS += {new_name}\n")
                f.write(f"{new_name}.file = {new_name}/{new_name}_Tests.pro\n")
            print(f"✅ Добавлена запись в tests.pro")
        else:
            print(f"⚠️ Тест '{new_name}' уже добавлен в tests.pro")
    else:
        print("❌ Файл 'tests.pro' не найден.")
        with open(tests_pro_path, "w", encoding="utf-8") as f:
            f.write(f"TEMPLATE = subdirs\n\nSUBDIRS += {new_name}\n")
            f.write(f"{new_name}.file = {new_name}/{new_name}_Tests.pro\n")

    print(f"✅ Тест '{new_name}' успешно создан.")
    print(f"📁 Файлы созданы в папке: {dst_dir}/")
    print(f"   - {new_name}_Tests.pro")
    print(f"   - {new_name}UnitTests.h")
    print(f"   - {new_name}UnitTests.cpp")

if __name__ == "__main__":
    create_new_test()