project(
  'och8S', 
  'c',
  version : '0.1',
  default_options : [
    'c_std=c18',
    'warning_level=3',
    'optimization=3',
  ]
)

add_project_arguments(
  [
    '-Wshadow',
  ], 
  language: 'c'
)

sdl2 = dependency('sdl2')

subdir('src')
