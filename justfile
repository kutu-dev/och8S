# Default recipe of the justfile
default rom: (run rom)

# Show this info message
help:
  just --list

# Setup the build environment
setup:
  rm -rf build
  meson setup build


# Compile och8S
compile:
  meson compile -C build

# Compile and run och8S
run rom: compile
  build/src/och8S {{rom}}

debug rom: compile
  build/src/och8S -ds {{rom}}

# Check the linting and formatting of the project
check:
  cppcheck src/ --check-level=exhaustive
  find src/*.c include/*.h | xargs clang-format -i

# See all the things that need to be done
todo:
  glow TODO.md
  rg TODO
