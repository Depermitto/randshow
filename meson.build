project(
  'randshow',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++17'],
)

catch2_dep = dependency('catch2')

test_main_exe = executable('test_main', 'tests/tests.cpp', dependencies: catch2_dep)

randshow_test_exe = executable(
  'randshow_test',
  'tests/randshow_test.cpp',
  dependencies: catch2_dep,
  objects: test_main_exe.extract_all_objects(),
)
test('randshow_test', randshow_test_exe)