/***
*crtassem.h - Libraries Assembly information
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file has information about Libraries Assembly version.
*
*       [Public]
*
****/

#pragma once

#pragma message ("Forceably setting current crt version so swig will recognize it when generation .cxx files.  Attempting to set it via -D in the swig command line doesn't have an affect.")
#define _BIND_TO_CURRENT_CRT_VERSION 1

#ifndef _VC_ASSEMBLY_PUBLICKEYTOKEN
#define _VC_ASSEMBLY_PUBLICKEYTOKEN "1fc8b3b9a1e18e3b"
#endif

#if !defined(_BIND_TO_CURRENT_VCLIBS_VERSION)
  #define _BIND_TO_CURRENT_VCLIBS_VERSION 0
#endif

#if !defined(_BIND_TO_CURRENT_CRT_VERSION)
  #if _BIND_TO_CURRENT_VCLIBS_VERSION
    #define _BIND_TO_CURRENT_CRT_VERSION 1
  #else
      #define _BIND_TO_CURRENT_CRT_VERSION 0
  #endif
#endif

#ifndef _CRT_ASSEMBLY_VERSION
#if _BIND_TO_CURRENT_CRT_VERSION
#define _CRT_ASSEMBLY_VERSION "9.0.30729.6161"
#else
#define _CRT_ASSEMBLY_VERSION "9.0.21022.8"
#endif
#endif

#ifndef __LIBRARIES_ASSEMBLY_NAME_PREFIX
#define __LIBRARIES_ASSEMBLY_NAME_PREFIX "Microsoft.VC90"
#endif
