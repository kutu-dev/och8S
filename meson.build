project(
  'och8S', 
  'c',
  version : '1.0.0',
  default_options : [
    'c_std=c23',
    'warning_level=3',
    'optimization=3',
    'werror=true',
  ]
)

add_project_arguments(
  [
    '-Wshadow',
  ], 
  language: 'c'
)

sdl2_dep = dependency('sdl2')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

include_dir = include_directories('include')

subdir('src')
