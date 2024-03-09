#!/bin/bash
set -x

clang -Wall -Wextra -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL ./raylib/raylib-5.0_macos/lib/libraylib.a -o paintxd src/paintxd.c