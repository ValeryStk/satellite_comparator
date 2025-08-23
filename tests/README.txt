Make new test instruction:

1. copy _empty_tets dir to this dir

2. rename it like you want to name future test (for example NEW_NAME)

3. edit tests.pro file
   - add the NEW_NAME of your folder to  SUBDIRS +=
   - add new line after it NEW_NAME.file = NEW_NAME/NEW_NAME_Tests.pro

4. open your NEW_NAME dir
   - rename pro file to NEW_NAME_Tests.pro
   - open and edit NEW_NAME_Tests.pro
   - edit TARGET to NewNameTests
   - add HEADERS you need
   - add SOURCES you need