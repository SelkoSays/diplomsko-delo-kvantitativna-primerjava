#!/usr/bin/env bash

FLAGS=" -std=c++20"

FLAGS=" -Wall -Wpedantic -Wextra"
FLAGS="$FLAGS -Wno-gnu-zero-variadic-macro-arguments -Wno-c++20-designator -Wno-unused-command-line-argument -Wno-unused-function"
FLAGS="$FLAGS -Werror=implicit-function-declaration"

# link math library
MATH=" -lm"

# DBG_FLAGS=" -fsanitize=leak,address,undefined"
DBG_FLAGS=" "

CFILES=(../c/utils/args_parser.c)
FILES=(main.cpp pool/pool.cpp random/random.cpp tracker/tracker.cpp actions/actions.cpp utils/progress.cpp)

CC=clang
# CC=gcc

CXX=clang++
# CXX=g++

mkdir -p build

# echo $CC $FLAGS $DBG_FLAGS ${FILES[@]} -o build/c-test $MATH $*
$CC -c $FLAGS $DBG_FLAGS ${CFILES[@]} -o build/c-obj.o $MATH $*
$CXX $FLAGS $DBG_FLAGS ${FILES[@]} build/c-obj.o -o build/cpp-test $MATH $*