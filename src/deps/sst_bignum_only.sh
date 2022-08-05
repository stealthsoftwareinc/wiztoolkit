#! /bin/bash

#
# Copyright (C) 2020 Stealth Software Technologies, Inc.
#
# Author: Kimberlee Model 2020-12-16
#

SST_COMMIT=e5cb49bc8c6f4d95f54cffc89c42d459b7caa3ec

show_help()
{
  echo "sst_bignum_only.sh [ --windows ] [ --nox86_64 ] [ --help ]"
  echo
  echo "  --windows    indicates that this is a windows and not posix (default) install"
  echo "  --nox86_64   indicates that the platform is x86, but not x86_64"
  echo "               other platforms are not currently supported"
  echo "  --clean      delete output folders before redownloading"
  echo "  --help       print the help text"
  echo
  echo "  configuration flags for presence of libcrypto (default yes) could be added"
  echo "  currently libcrypto is assumed/required to be installed already"
}

POSIX="1"
WINDOWS="0"
X86="1"
X86_64="1"

while [ $# -gt "0" ]
do
  case $1 in
    "--windows")
      POSIX="0"
      WINDOWS="1"
      ;;
    "--nox86_64")
      X86_64="0"
      ;;
    "--clean")
      rm -rf src/
      rm -rf include/
      ;;
    "--help")
      show_help
      exit
      ;;
    *)
      echo "unrecognized ${1}"
      show_help
      exit 1
  esac
  shift
done

mkdir -p include/sst/catalog
mkdir -p src/sst/

# $1 local prefix
# $2 remote prefix
# $3 file name
get_sst_file()
{
  wget -O "${1}/${3}" "https://github.com/stealthsoftwareinc/sst/raw/${SST_COMMIT}/${2}/${3}"
}

get_h()
{
  get_sst_file "include/" "src/c_cpp/include/" "${1}"
}

get_c()
{
  get_sst_file "src/" "src/c_cpp/lib/" "${1}"
}

# get the license
get_sst_file . "" COPYING 

get_h sst/catalog/bignum.hpp
get_h sst/catalog/bignum_error.hpp
get_c sst/bignum.cpp

get_h sst/catalog/SST_CPP_CONSTEXPR.hpp
get_h sst/catalog/SST_CPP_OR_LATER.hpp
get_h sst/catalog/SST_CPP_VALUE.hpp
get_h sst/catalog/SST_CPP_INLINE.hpp
get_h sst/catalog/SST_PUBLIC_CPP_CLASS.hpp
get_h sst/catalog/SST_PUBLIC_CPP_FUNCTION.hpp
get_h sst/catalog/SST_CONSTEXPR_ASSERT.hpp
get_h sst/catalog/SST_STATIC_ASSERT.hpp

get_h sst/catalog/can_represent_all.hpp
get_h sst/catalog/char_bit.hpp
get_h sst/catalog/checked.hpp
get_h sst/catalog/checked_cast.hpp
get_h sst/catalog/checked_error.hpp
get_h sst/catalog/integer_rep.hpp
get_h sst/catalog/is_negative.hpp
get_h sst/catalog/to_unsigned.hpp
get_h sst/catalog/uchar_max.hpp
get_h sst/catalog/unsigned_ge.hpp
get_h sst/catalog/unsigned_gt.hpp
get_h sst/catalog/unsigned_le.hpp
get_h sst/catalog/unsigned_lt.hpp
get_h sst/catalog/unsigned_eq.hpp
get_h sst/catalog/unsigned_ne.hpp
get_h sst/catalog/value_bits.hpp
get_h sst/catalog/perfect_ge.hpp
get_h sst/catalog/perfect_gt.hpp
get_h sst/catalog/perfect_lt.hpp
get_h sst/catalog/perfect_le.hpp
get_h sst/catalog/perfect_eq.hpp
get_h sst/catalog/perfect_ne.hpp
get_h sst/catalog/integer_promote.hpp
get_h sst/catalog/integer_promotes.hpp
get_h sst/catalog/promote_unsigned.hpp
get_h sst/catalog/promote_with_sign.hpp
get_h sst/catalog/type_max.hpp
get_h sst/catalog/type_min.hpp
get_h sst/catalog/width_bits.hpp
get_h sst/catalog/is_bool.hpp
get_h sst/catalog/remove_cvref.hpp

get_h sst/language.h
get_h sst/limits.h
get_h sst/type.h
get_h sst/type.hpp
get_h sst/integer.h

get_h sst/config.h.im.in

cat include/sst/config.h.im.in \
  | sed "s/\\@HAVE_LIB_CRYPTO\\@/1/" \
  | sed "s/\\@HAVE_POSIX\\@/${POSIX}/" \
  | sed "s/\\@HAVE_WINDOWS\\@/${WINDOWS}/" \
  | sed "s/\\@HAVE_X86\\@/${X86}/" \
  | sed "s/\\@HAVE_X86_64\\@/${X86_64}/" \
  | sed "s/\\@WITH_BUILD_GROUP_CPP_AUTOTOOLS\\@/1/" > include/sst/config.h

rm include/sst/config.h.im.in
