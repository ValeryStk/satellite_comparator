# SCRIPT FOR RESOLVING ALL DEPENDENCIES
"""

@Author: Valery Stanchyk
@Date: 2024/09/25

"""

import os
import shutil
import py7zr

# CONSTANTS
DEPS_EXE_FILE = "deps_exe"
DEPS_ONE_LEVEL_ABOVE_EXE_FILE = "deps_above_exe"
DEPS_ONLY_FOR_DEBUG = "deps_only_for_debug"
DEPS_ONLY_FOR_RELEASE = "deps_only_for_release"
RELEASE_DIR_NAME = "release"
DEBUG_DIR_NAME = "debug"
LEVELS_UP_TO_BUILD_DIRS = "../../"
ARHIV_FILES = [".7z"]
EXTRA_DIR_FOR_OUT = "/core"

# ARRAY FOR OUT DIRS
out_dir_list = []

# COMMON FUNCTIONS
def copytree(src, dst, destination_path, symlinks=False, ignore=None):
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            if os.path.exists(d):
                continue
            shutil.copytree(s, d, symlinks, ignore)
        else:
            if os.path.exists(d):
                continue
            if is_file_arhiv(s):
                print(f"UNPACKING  {s} ---> {dst}")
                unpack(s,dst)
                continue
            shutil.copy2(s, d)

def make_release_or_debug_path(out):
        r_d = '/'
        if RELEASE_DIR_NAME in out.lower():
            r_d = r_d + RELEASE_DIR_NAME
        if DEBUG_DIR_NAME in out.lower():
            r_d = r_d + DEBUG_DIR_NAME
        return r_d

def is_file_arhiv(file):
    for check in ARHIV_FILES:
        if check in file.lower():
           return True
    return False

def unpack(file,out):
   archive = py7zr.SevenZipFile(file, mode='r')
   archive.extractall(out)
   archive.close() 

def resolve_deps(files_list, DEPS_FOLDER_NAME:str, IS_EXE_LEVEL:bool):
 for file in files_list:
     file = DEPS_FOLDER_NAME + "/" + file
     print(file)
     for out in out_dir_list:
        r_d = ""
        if IS_EXE_LEVEL:
           r_d = make_release_or_debug_path(out)
           if r_d == '/':
              print('RELEASE DEBUG UNEXPECTED ERROR',out)
              continue
           if DEPS_FOLDER_NAME == DEPS_ONLY_FOR_RELEASE:
               if r_d == '/debug':
                   continue
           if DEPS_FOLDER_NAME == DEPS_ONLY_FOR_DEBUG:
               if r_d == '/release':
                   continue
        destination_path = LEVELS_UP_TO_BUILD_DIRS + out + EXTRA_DIR_FOR_OUT + r_d
        if os.path.isfile(file):
           if is_file_arhiv (file):
              unpack(file, destination_path)
              continue
           print("file exists: " + file)
           print(f"copy exe file deps {file} to: -->", destination_path)
           shutil.copy(file, destination_path)
        if os.path.isdir(file):
           print(f"copy dir deps {file} to: --> {destination_path}")
           base_name_ = os.path.basename(file)
           print("BASE NAME: -- >" + base_name_)
           full_path = destination_path + "/" + base_name_
           if not os.path.exists(full_path):
               os.mkdir(full_path)
           copytree(file, full_path, destination_path)


# PREPARATIONS ########################################################################################################

print('STEP 1 - Create deps directories list and files list')
files_in_exe_list = os.listdir(DEPS_EXE_FILE)
files_in_above_exe_list = os.listdir(DEPS_ONE_LEVEL_ABOVE_EXE_FILE)
files_only_for_debug = os.listdir(DEPS_ONLY_FOR_DEBUG)
files_only_for_release = os.listdir(DEPS_ONLY_FOR_RELEASE)


print('\nSTEP 2 - Create build directories list')
build_dirs_list = os.listdir(LEVELS_UP_TO_BUILD_DIRS)
for dir in build_dirs_list:
    if not RELEASE_DIR_NAME  in dir.lower() and not DEBUG_DIR_NAME in dir.lower():
        print('IGNORE THIS DIR (no release and debug word in its name) -->', dir)
        continue
    if os.path.isfile(dir):
        continue
    print("BUILD folder: " + dir)
    common_path = LEVELS_UP_TO_BUILD_DIRS + dir + EXTRA_DIR_FOR_OUT
    full_release_path = common_path + "/" + RELEASE_DIR_NAME
    full_debug_path = common_path + "/" + DEBUG_DIR_NAME
    if not os.path.exists(common_path):
        print(f"!!! WARNING EXTRA DIR FOR OUT DOESN'T EXISTS !!! ---> {common_path}")
        continue
    print ("FULL_PATH: " + full_release_path)
    if not os.path.exists(full_release_path):
       os.mkdir(full_release_path)
    if not os.path.exists(full_debug_path):
       os.mkdir(full_debug_path)
    out_dir_list.append(dir)


# RESOLVING DEPS SECTION ##############################################################################################

print('STEP 3 - Copy files to build dirs with .EXE')
resolve_deps(files_in_exe_list, DEPS_EXE_FILE, True)

print('STEP 4 - Copy files to build one level above exe dir')
resolve_deps(files_in_above_exe_list, DEPS_ONE_LEVEL_ABOVE_EXE_FILE, False)

print('STEP 5 - Copy files only for debug to exe dir')
resolve_deps(files_only_for_debug, DEPS_ONLY_FOR_DEBUG, True)

print('STEP 6 - Copy files only for release to exe dir')
resolve_deps(files_only_for_release, DEPS_ONLY_FOR_RELEASE, True)

