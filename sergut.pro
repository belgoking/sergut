TEMPLATE = subdirs

SUBDIRS += my_library

withTest {
SUBDIRS += my_test
}

withBenchmark {
SUBDIRS += my_benchmark
}

my_library.subdir = lib
my_test.subdir = test
my_test.depends = my_library
my_benchmark.subdir = benchmark
my_benchmark.depends = my_library
