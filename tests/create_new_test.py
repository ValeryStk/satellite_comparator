import os
import shutil

def rename_file(old_path, new_path):
    try:
        os.rename(old_path, new_path)
        print(f"✅ Файл успешно переименован:\n{old_path} → {new_path}")
    except FileNotFoundError:
        print(f"❌ Файл не найден: {old_path}")
    except FileExistsError:
        print(f"⚠️ Файл с именем {new_path} уже существует.")
    except Exception as e:
        print(f"⚠️ Произошла ошибка: {e}")


def rename_class_in_file(file_path, old_name, new_name):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()

        # Заменяем все вхождения старого имени на новое
        updated_content = content.replace(old_name, new_name)

        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(updated_content)

        print(f"✅ Класс '{old_name}' успешно переименован в '{new_name}' в файле {file_path}")
    except FileNotFoundError:
        print(f"❌ Файл {file_path} не найден.")
    except Exception as e:
        print(f"⚠️ Произошла ошибка: {e}")


def create_new_test():
    # Ввод имени нового теста
    new_name = input("Введите имя нового теста: ").strip()

    # Шаг 1: Копируем _empty_tests
    src_dir = "_empty_tests"
    dst_dir = new_name
    if not os.path.exists(src_dir):
        print(f"❌ Папка '{src_dir}' не найдена.")
        return
    shutil.copytree(src_dir, dst_dir)

    # Шаг 2 и 3: Редактируем tests.pro
    tests_pro_path = "tests.pro"
    if not os.path.exists(tests_pro_path):
        print("❌ Файл 'tests.pro' не найден.")
        return

    with open(tests_pro_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    with open(tests_pro_path, "a", encoding="utf-8") as f:
        f.write(f"\nSUBDIRS += {new_name}\n")
        f.write(f"{new_name}.file = {new_name}/{new_name}_Tests.pro\n")


    # Шаг 4: Редактируем .pro файл внутри новой директории
    old_pro_path = os.path.join(dst_dir, "empty_tests.pro")
    new_pro_path = os.path.join(dst_dir, f"{new_name}_Tests.pro")
    print(f"Переименование: {old_pro_path} → {new_pro_path}")
    print(f"Существует ли исходный файл? {os.path.exists(old_pro_path)}")
    print(f"Существует ли целевой файл? {os.path.exists(new_pro_path)}")

    os.rename(old_pro_path, new_pro_path)

    with open(new_pro_path, "w", encoding="utf-8") as f:
        f.write("QT += testlib core\n")
        f.write(f"TARGET = {new_name}_Tests\n")
        f.write(f"HEADERS += {new_name}_UnitTests.h\\\n")
        f.write(f"\nSOURCES += {new_name}_UnitTests.cpp\\\n")
    rename_class_in_file(f"{dst_dir}/UnitTests.h",'UnitTests',f"{new_name}_UnitTests")
    rename_class_in_file(f"{dst_dir}/UnitTests.cpp",'UnitTests',f"{new_name}_UnitTests")
    rename_file(f"{dst_dir}/UnitTests.cpp",f"{dst_dir}/{new_name}_UnitTests.cpp")
    rename_file(f"{dst_dir}/UnitTests.h",f"{dst_dir}/{new_name}_UnitTests.h")
    print(f"✅ Тест '{new_name}' успешно создан.")

if __name__ == "__main__":
    create_new_test()
