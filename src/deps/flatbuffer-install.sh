#! /bin/bash

# Copyright (C) 2021 Stealth Software Technologies, Inc.

ENABLE_FLATBUFFER="$1"
CMAKE_CMD="$2"

if [ "${ENABLE_FLATBUFFER}" != 1 ] ; then
  exit 0
fi

if [-d flatbuffer/ ] ; then
  rm -rf flatbuffer/
fi

wget -O flatbuffer.tar.gz \
  https://github.com/google/flatbuffers/archive/refs/tags/v2.0.0.tar.gz

tar -xzf flatbuffer.tar.gz
mv flatbuffers-2.0.0/ flatbuffer/
rm flatbuffer.tar.gz

cd flatbuffer/
mkdir -p build/
cd build/
"${CMAKE_CMD}" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
  -DFLATBUFFERS_BUILD_TESTS=OFF \
  -DFLATBUFFERS_BUILD_FLATLIB=OFF \
  -DFLATBUFFERS_BUILD_FLATHASH=OFF \
  ../
make -j4
