project('egm', 'cpp', version : '0.0.1', default_options: ['cpp_std=c++23'])
include_dir = include_directories('include')
if (get_option('buildTests'))
  gtest = dependency('gtest', main : true, required : false)
  yaml_cpp = dependency('yaml-cpp')
  egm_tests = executable('egm_tests', 
    'src/test/managerTest.cpp', 'src/test/setTest.cpp',
    include_directories: include_dir, 
    dependencies: [gtest, yaml_cpp],
    cpp_args: ['-fsanitize=address', '-fno-omit-frame-pointer'],
    link_args: '-fsanitize=address',

  )
  test('egm_tests', egm_tests)
endif
