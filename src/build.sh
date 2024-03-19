#!/bin/bash

to_upper() {
    return $1 | tr '[:lower:]' '[:upper:]'
}

mkdir -p ../build
pushd ../build

arg_renderer = to_upper $1
echo "${arg_renderer}"
if [ "${arg_renderer}" != "METAL" -a "${arg_renderer}" != "OPENGL"]; then
    echo "Renderer has to be either metal or opengl, was " + $1
    exit
fi

case $(uname) in
    Darwin) # Mac
        clang++ -std=c++20 -DPERF_DIAGNOSTIC=0 -DDEBUG=1 -DOPENGL ../src/sdl_platform.cpp -o game -O0 -g `sdl2-config --cflags --libs` -framework Foundation -framework QuartzCore -framework Metal -I../src/include/glad/include
        ;;
    *) # Linux
        g++ -std=c++20 -DPERF_DIAGNOSTIC=0 -DDEBUG=1 -DOPENGL ../src/sdl_platform.cpp -o game -O0 -g `sdl2-config --cflags --libs` -lGL -lGLEW
        ;;
esac
# g++ ../src/sdl_platform.cpp -o game -O0 -g `sdl2-config --cflags --libs` -lGL -lGLEW

popd
