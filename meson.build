project(
  'randshow',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++11'],
)

executable('randshow', 'app/main.cpp')

executable('gen_binary_data', 'tests/gen_binary_data.cpp')
randshow_test = find_program('tests/randshow_test.sh')
test('randshow_test', randshow_test, timeout: -1)