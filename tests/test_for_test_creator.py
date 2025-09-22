import unittest
import tempfile
import os
from unittest.mock import patch
from create_new_test import test_name_checker, create_new_test

class TestForNameChecker(unittest.TestCase):
    @patch('builtins.input', side_effect=['', '1Test', 'Test@123', 'Тест', 'Valid_Name'])
    def test_valid_name(self, mock_input):
        result = test_name_checker()
        self.assertEqual(result, 'Valid_Name')

#проверка для функци crate_new_test        
class TestCreateNewTest(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.original_cwd = os.getcwd()
        os.chdir(self.temp_dir.name)

    def tearDown(self):
        os.chdir(self.original_cwd)
        self.temp_dir.cleanup()

    #проверка создаются ли файлы
    @patch('builtins.input', return_value='MyTest')
    def test_file_create(self, mock_input):
        create_new_test()

        test_dir = 'MyTest'
        self.assertTrue(os.path.isdir(test_dir))

        pro_path = os.path.join(test_dir, 'MyTest_Tests.pro')
        h_path = os.path.join(test_dir, 'MyTestUnitTests.h')
        cpp_path = os.path.join(test_dir, 'MyTestUnitTests.cpp')

        self.assertTrue(os.path.exists(pro_path))
        self.assertTrue(os.path.exists(h_path))
        self.assertTrue(os.path.exists(cpp_path))

        with open(pro_path, encoding='utf-8') as f:
            self.assertIn('TARGET = MyTestUnitTests', f.read())

        with open(h_path, encoding='utf-8') as f:
            self.assertIn('#ifndef MYTESTUNITTESTS_H', f.read())

        with open(cpp_path, encoding='utf-8') as f:
            self.assertIn('QTEST_MAIN(MyTestUnitTests)', f.read())
            
    #проверка файла tests.pro
    @patch('builtins.input', return_value='AnotherTest')
    def test_tests_pro_update(self, mock_input):
        with open('tests.pro', 'w', encoding='utf-8') as f:
            f.write('TEMPLATE = subdirs\n')

        create_new_test()

        with open('tests.pro', encoding='utf-8') as f:
            content = f.read()
            self.assertIn('SUBDIRS += AnotherTest', content)
            self.assertIn('AnotherTest.file = AnotherTest/AnotherTest_Tests.pro', content)

    @patch('builtins.input', return_value='DuplicateTest')
    def test_tests_pro_duplicate(self, mock_input):
        with open('tests.pro', 'w', encoding='utf-8') as f:
            f.write('SUBDIRS += DuplicateTest\n')

        create_new_test()

        with open('tests.pro', encoding='utf-8') as f:
            content = f.read()
            self.assertEqual(content.count('SUBDIRS += DuplicateTest'), 1)

if __name__ == "__main__":
    unittest.main()
