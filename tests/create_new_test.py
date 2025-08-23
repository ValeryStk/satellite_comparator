import os
import shutil

def create_new_test():
    # Ввод имени нового теста
    new_name = input("Введите имя нового теста: ").strip()

    # Шаг 1: Копируем _empty_tets
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

    with open(tests_pro_path, "w", encoding="utf-8") as f:
        for line in lines:
            f.write(line)
            if line.strip().startswith("SUBDIRS +="):
                f.write(f"SUBDIRS += {new_name}\n")
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
        f.write(f"TARGET = {new_name}Tests\n")
        f.write("HEADERS += UnitTests.h\\\n")
        f.write("\nSOURCES += UnitTests.cpp\\\n")

    print(f"✅ Тест '{new_name}' успешно создан.")

if __name__ == "__main__":
    create_new_test()
