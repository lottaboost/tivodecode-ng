
#ifndef __TIVO_TYPES_HXX__
#define __TIVO_TYPES_HXX__
//
//#include <stddef.h>

#ifdef HAVE_CONFIG_H
#include "tdconfig.h"
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

typedef unsigned int        UINT32;
typedef int                 INT32;
typedef unsigned short      UINT16;
typedef short               INT16;
typedef unsigned char       UINT8;
typedef char                INT8;
typedef char                CHAR;
typedef char                BOOL;

#define TRUE  1
#define FALSE 0

#define PRINT_QUALCOMM_MSG() fprintf (stderr, "Encryption by QUALCOMM ;)\n\n")

#endif /* TIVO_TYPES_HXX__ */



/* vi:set ai ts=4 sw=4 expandtab: */

