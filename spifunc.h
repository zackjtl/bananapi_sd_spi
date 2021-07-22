#ifndef __SPIFUNC_H
#define __SPIFUNC_H

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
//-------------------------------------------------------------------------
// Data Struct
//-------------------------------------------------------------------------
typedef struct __CSPIDEV {
    int fd;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
} CSPIDEV, *pCSPIDEV;
//-------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------
extern int SPI_DataTransfer( uint8_t value );
extern int SPI_SetTransferSpeed( void );
extern int SPI_SetBitsPerWord( void );
extern int SPI_SetMode( void );
extern int SPI_WriteByte( uint8_t buf );
extern int Enabled_Default_Mode( uint8_t ModeType );
extern int SPI_Cs_Deselect();
extern int SPI_Cs_Select();

extern CSPIDEV device;

#endif
