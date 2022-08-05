#! /bin/bash

# Copyright (C) 2020 Stealth Software Technologies, Inc.

ENABLE_ANTLR="$1"
if [ "${ENABLE_ANTLR}" != 1 ] ; then
  exit 0
fi

if [ -d antlr4 ] ; then
  rm -rf antlr4
fi

mkdir -p antlr4
cd antlr4

wget -O antlr-tool.jar \
  https://www.antlr.org/download/antlr-4.8-complete.jar

wget -O antlr-cpp.zip \
  https://www.antlr.org/download/antlr4-cpp-runtime-4.8-source.zip
unzip antlr-cpp.zip
