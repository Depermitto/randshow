project(
  'randshow',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++11'],
)

executable('randshow', 'app/main.cpp')

randshow_test = executable(
  'randshow_test',
  'tests/randshow_test.cpp',
  objects: executable('catch_main', 'tests/tests.cpp').extract_all_objects(),
)
test('randshow_test', randshow_test, timeout: -1)

# Quality test
executable('PractRand-randshow', 'tests/PractRand-randshow.cpp')