project(
  'randshow',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++11'],
)

incdir = include_directories('include')

# App
executable(
  'randshow',
  'app/main.cpp',
  include_directories: incdir,
)

# Tests
randshow_test = executable(
  'randshow_test',
  'tests/randshow_test.cpp',
  objects: executable('catch_main', 'tests/tests.cpp').extract_all_objects(),
  include_directories: incdir,
)
test('randshow_test', randshow_test, timeout: -1)

# PractRand
executable(
  'PractRand-randshow',
  'tests/PractRand-randshow.cpp',
  include_directories: incdir,
)