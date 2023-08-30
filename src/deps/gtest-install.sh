#! /bin/bash

# Copyright (C) 2020 Stealth Software Technologies, Inc.

ENABLE_GTEST="$1"

if [ "${ENABLE_GTEST}" != 1 ] ; then
  exit 0
fi

if [ -d gtest ] ; then
  rm -rf gtest
fi

mkdir gtest

wget -O gtest.tar.gz \
  https://github.com/google/googletest/archive/release-1.10.0.tar.gz

tar --strip-components=1 --directory=gtest -xzf gtest.tar.gz
rm gtest.tar.gz
