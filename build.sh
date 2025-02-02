#!/bin/zsh
time gcc -Wall -Wextra -o body body.c -L. -lraylib -framework IOKit -framework Cocoa -framework OpenGL
