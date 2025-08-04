#!/bin/sh
gcc main.c -o wsimplecal `pkg-config --libs --cflags gtk4 gtk4-layer-shell-0`
