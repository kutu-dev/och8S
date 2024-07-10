sources = files('main.c', 'render.c', 'virtual-machine.c', 'keys.c', 'logging.c', 'opcodes.c', 'audio.c', 'save-state.c')

exe = executable(
  'och8S',
  sources,
  dependencies: [sdl2_dep, m_dep],
  include_directories: include_dir
)
