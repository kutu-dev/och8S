project(
  'och8S', 
  'c',
  version : '0.1',
  default_options : [
    'c_std=c23',
    'warning_level=3',
    'optimization=3',
    ##'werror=true',
  ]
)

add_project_arguments(
  [
    '-Wshadow',
  ], 
  language: 'c'
)

sdl2_dep = dependency('sdl2')

include_dir = include_directories('include')

subdir('src')
