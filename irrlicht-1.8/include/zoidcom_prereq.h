/****************************************
* zoidcom_prereq.h
* prerequisites
*
* This file is part of the "Zoidcom Automated Networking System" application library.
* Copyright (C)2002-2007 by Joerg Rueppel. See documentation for copyright and licensing details.
*****************************************/

#ifndef _ZOIDCOM_PREREQ_H
#define _ZOIDCOM_PREREQ_H

/** \file zoidcom_prereq.h
*/

#include <cassert>
#include <cstddef>
#include <cstring>

#ifdef WIN32
  #define ZCOM_PLATFORM_WIN32
#endif

#ifdef __linux__
  #define ZCOM_PLATFORM_LINUX
#endif

#if __APPLE__
  #define ZCOM_PLATFORM_MAC
#endif

#ifdef _BIG_ENDIAN
  #define ZCOM_BIG_ENDIAN
#else
  #define ZCOM_LITTLE_ENDIAN
#endif

#if WIN32 && __MINGW32__ && !ZCOM_BUILD
  #define ZCOM_API
  #define ZCOM_TAPI
#elif WIN32 && __MINGW32__ && ZCOM_BUILD
  #define ZCOM_API  __declspec(dllexport)
  #define ZCOM_TAPI __declspec(dllexport)
#elif ZCOM_BUILD && WIN32
  #define ZCOM_API __declspec(dllexport)
  #define ZCOM_TAPI
#elif WIN32
  #define ZCOM_API __declspec(dllimport)
  #define ZCOM_TAPI
#elif (defined ZCOM_PLATFORM_LINUX || defined ZCOM_PLATFORM_MAC) && ZCOM_BUILD
  #define ZCOM_API __attribute__ ((visibility("default")))
  #define ZCOM_TAPI __attribute__ ((visibility("default")))
#else
  #define ZCOM_API
  #define ZCOM_TAPI
#endif

#ifndef NULL
  #define NULL 0
#endif

// data types
/// unsigned 8 bit
typedef unsigned char        zU8;
#define zU8_MAX              UCHAR_MAX
#define zU8_MIN              0
/// signed 8 bit
typedef signed char          zS8;
#define zS8_MAX              SCHAR_MAX
#define zS8_MIN              SCHAR_MIN
/// unsigned 16 bit
typedef unsigned short       zU16;
#define zU16_MAX             USHRT_MAX
#define zU16_MIN             0
/// signed 16 bit
typedef signed short         zS16;
#define zS16_MAX             SHRT_MAX
#define zS16_MIN             SHRT_MIN
/// unsigned 32 bit
typedef unsigned int         zU32;
#define zU32_MAX             UINT_MAX
#define zU32_MIN             0
/// signed 32 bit
typedef signed int           zS32;
#define zS32_MAX             INT_MAX
#define zS32_MIN             INT_MIN
/// unsigned 64 bit
typedef unsigned long long   zU64;
#define zU64_MAX             0xffffffffffffffff
#define zU64_MIN             0
/// signed 64 bit
typedef signed long long     zS64;
#define zS64_MAX             0x7fffffffffffffff
#define zS64_MIN             0x8000000000000000
/// float
typedef float                zFloat;
/// double
typedef double               zDouble;

#endif
