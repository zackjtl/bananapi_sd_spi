#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

//---------------------------------------------------------------
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))





#define ALLOC_MEMORY( type, size )      malloc( size * sizeof(type) )
#define FREE_MEMORY( mem )              free( mem )
#define FILL_MEMORY( mem, data, size )  memset( mem, data, size )


//------------------------------------------------------------------------------

typedef union __LABADDR {
    uint32_t Addr;
    struct {
        uint8_t B3;
        uint8_t B2;
        uint8_t B1;
        uint8_t B0;
    };
} LBAADDR, *pLBAADDR;

typedef struct __ACCESSARGV {
    LBAADDR 	StartLba;
    uint16_t	ReadCnt;
} ACCESSARGV, *pACCESARGV;

#endif