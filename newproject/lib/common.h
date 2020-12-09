#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <fcntl.h>

#define DBG_INFO
#ifdef DBG_INFO
#define DBG(fmt, ...) printf("%s()[%d] " fmt "\n",__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define DBG(fmt, ...) printf("" fmt "\n",##__VA_ARGS__)
#endif


#if __WORDSIZE == 64
typedef long int            INT_PTR;
typedef unsigned long       U_PTR;
#else
typedef int                 INT_PTR;
typedef unsigned int        U_PTR;
#endif

typedef char                INT8;
typedef unsigned char       UINT8;
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef INT_PTR             LONG;
typedef U_PTR               ULONG;
typedef long long           LLONG;
typedef unsigned long long  ULLONG;
typedef LLONG               INT64;
typedef ULLONG              UINT64;
typedef void                VOID;
typedef double              DOUBLE;
typedef float               FLOAT;