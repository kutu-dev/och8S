# Default recipe of the justfile
default: run

# Show this info message
help:
  just --list

# Setup the build environment
setup:
  rm -rf build
  meson setup build

# Compile and run och8S
run:
  meson compile -C build
  build/src/och8S

# Check the linting and formatting of the project
check:
  cppcheck src/
  find src/*.c include/*.h | xargs clang-format -i
