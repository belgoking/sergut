TEMPLATE = subdirs

SUBDIRS += my_test my_library
my_test.subdir = test
my_test.depends = my_library
my_library.subdir = lib
